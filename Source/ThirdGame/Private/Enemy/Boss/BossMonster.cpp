// =========================================================================================
// BossMonster.cpp
//
// [파일 역할]
// AEnemy를 상속받는 보스 몬스터 클래스입니다.
// 1페이즈 → 2페이즈(분노 페이즈) 전환, 전용 HP 바 UI, 점프 공격, 회전 공격 등
// 보스 고유의 전투 패턴을 구현합니다.
//
// [페이즈 전환 흐름]
// HP 0 도달(1페이즈) → EnterRagePhase() → 무적 + 변신 몽타주 재생
// → EndRageTransformation() → HP 전체 회복 + AI 재시작 + 2페이즈 시작
// =========================================================================================

#include "Enemy/Boss/BossMonster.h"
#include "Net/UnrealNetwork.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enemy/EnemyHPBarWidget.h"
#include "Enemy/Boss/BossHPWidget.h"
#include "Perception/AIPerceptionComponent.h"
#include "MyCharacter.h"
#include "TimerManager.h"

// 복제할 변수를 등록합니다.
void ABossMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABossMonster, bIsRagePhase);
	DOREPLIFETIME(ABossMonster, bIsInvincible);
	DOREPLIFETIME(ABossMonster, bIsJumpAttacking);
	DOREPLIFETIME(ABossMonster, CachedImpactLocation);
}

// 보스 전용 초기화 — 부모(AEnemy) 기본값을 유지합니다.
ABossMonster::ABossMonster()
{
}

// 무기 메시 탐색, 보스 HP 바 UI 생성, 2페이즈 이동 속도 캐싱, Perception 콜백 추가 바인딩을 수행합니다.
void ABossMonster::BeginPlay()
{
	Super::BeginPlay();

	// 블루프린트에서 붙인 무기 스태틱 메시를 이름으로 탐색해 캐싱합니다.
	TArray<UStaticMeshComponent*> StaticMeshes;
	GetComponents<UStaticMeshComponent>(StaticMeshes);

	for (UStaticMeshComponent* MeshComp : StaticMeshes)
	{
		if (!MeshComp) continue;

		if (MeshComp->GetName().Contains(TEXT("Weapon")))
		{
			WeaponMesh = MeshComp;
			break;
		}
	}

	// 보스 전용 HP 바 위젯을 생성해 둡니다 (AddToViewport는 전투 시작 시 수행).
	if (BossHPWidgetClass)
	{
		BossUI = CreateWidget<UBossHPWidget>(GetWorld(), BossHPWidgetClass);
	}

	// DataTable에서 2페이즈 이동 속도를 미리 캐싱합니다.
	if (EnemyDataTable && !EnemyRowName.IsNone())
	{
		FEnemyData* Row = EnemyDataTable->FindRow<FEnemyData>(EnemyRowName, TEXT("BossBeginPlay"));
		if (Row && Row->Phase2MoveSpeed > 0.0f)
		{
			Phase2MoveSpeed = Row->Phase2MoveSpeed;
		}
	}

	// 보스 전용 Perception 콜백을 추가로 바인딩합니다
	// (베이스 클래스의 OnTargetDetected와 동시에 동작 — 블랙보드 세팅은 베이스가 처리).
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ABossMonster::OnBossTargetDetected);
	}
}

// 보스 전용 Perception 콜백과 HP 바 위젯을 정리합니다.
void ABossMonster::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(this, &ABossMonster::OnBossTargetDetected);
	}

	if (BossUI)
	{
		BossUI->RemoveFromParent();
	}

	Super::EndPlay(EndPlayReason);
}

// 플레이어를 처음 감지하면 전투 시작 상태로 전환하고 모든 클라이언트에 HP 바를 표시합니다.
void ABossMonster::OnBossTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !Actor->IsA<AMyCharacter>()) return;
	if (!HasAuthority()) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		// 이미 전투 중이면 중복 호출을 방지합니다.
		if (bIsEngaged) return;

		bIsEngaged = true;
		Multicast_ShowBossUI();
	}
}

// 2페이즈 복제 콜백: 변신 진입 시 HP 바를 갱신합니다. 이름 변경은 변신 완료 후 Multicast_OnRageComplete에서 수행합니다.
void ABossMonster::OnRep_bIsRagePhase()
{
	if (BossUI)
	{
		BossUI->UpdateHP(CurrentHP, MaxHP);
	}
}

// 점프 공격 상태 복제 콜백: 착지 시 클라이언트의 중력 스케일을 복구합니다.
void ABossMonster::OnRep_bIsJumpAttacking()
{
	if (!bIsJumpAttacking)
	{
		GetCharacterMovement()->GravityScale = 1.0f;
	}
}

// 모든 클라이언트에서 보스 HP 바를 처음 표시합니다.
void ABossMonster::Multicast_ShowBossUI_Implementation()
{
	if (BossUI)
	{
		BossUI->UpdateHP(CurrentHP, MaxHP);
		BossUI->UpdateBossName(Phase1Name);
		BossUI->AddToViewport();
	}
}

// 모든 클라이언트에서 변신 몽타주를 재생하고 HP 바를 갱신합니다.
void ABossMonster::Multicast_EnterRagePhase_Implementation()
{
	if (!HasAuthority() && TransformationMontage)
	{
		PlayAnimMontage(TransformationMontage);
	}

	if (BossUI)
	{
		BossUI->UpdateHP(CurrentHP, MaxHP);
	}
}

// 모든 클라이언트의 보스 HP 바를 갱신합니다.
void ABossMonster::Multicast_UpdateBossHP_Implementation(float NewHP, float NewMaxHP)
{
	if (BossUI)
	{
		BossUI->UpdateHP(NewHP, NewMaxHP);
	}
}

// 변신 애니메이션 완료 후 모든 클라이언트의 HP 바 이름과 수치를 갱신합니다.
void ABossMonster::Multicast_OnRageComplete_Implementation(float NewHP, float NewMaxHP)
{
	if (BossUI)
	{
		BossUI->UpdateBossName(Phase2Name);
		BossUI->UpdateHP(NewHP, NewMaxHP);
	}
}

// 모든 클라이언트에서 경고 데칼을 스폰합니다.
void ABossMonster::Multicast_SpawnWarningDecal_Implementation(FVector DecalLocation)
{
	if (!WarningDecalMaterial) return;

	WarningDecal = UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(),
		WarningDecalMaterial,
		FVector(WarningDecalRadius),
		DecalLocation,
		FRotator(-90.f, 0.f, 0.f));
}

// 모든 클라이언트에서 경고 데칼을 제거합니다.
void ABossMonster::Multicast_RemoveWarningDecal_Implementation()
{
	if (WarningDecal)
	{
		WarningDecal->DestroyComponent();
		WarningDecal = nullptr;
	}
}

// 점프 공격 중 하강 시 중력을 강화해 빠르게 낙하하도록 처리합니다.
void ABossMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsJumpAttacking)
	{
		// 최고점을 찍고 하강하기 시작하면(Z 속도 음수) 중력을 강화합니다.
		if (GetVelocity().Z < 0.f)
		{
			GetCharacterMovement()->GravityScale = 6.0f;
		}
	}
}

// 현재 미사용. 페이즈 전환 조건 확인 로직을 여기에 추가할 수 있습니다.
void ABossMonster::CheckPhaseTransition()
{
}

// 1페이즈 HP가 0에 도달하면 호출되어 무적 상태로 변신 몽타주를 재생합니다.
void ABossMonster::EnterRagePhase()
{
	bIsRagePhase = true;

	// 점프 공격 도중 페이즈가 전환되면 점프를 즉시 중단하고 상태를 초기화합니다.
	if (bIsJumpAttacking)
	{
		bIsJumpAttacking = false;

		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->GravityScale = 1.0f;

		if (JumpAttackMontage)
		{
			StopAnimMontage(JumpAttackMontage);
		}

		// 착지 없이 전환됐으므로 Landed()가 호출되지 않아 경고 데칼을 모든 클라이언트에서 제거합니다.
		Multicast_RemoveWarningDecal();
	}

	// 2페이즈 이동 속도를 적용합니다.
	if (Phase2MoveSpeed > 0.0f && GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Phase2MoveSpeed;
	}

	bIsInvincible = true;

	if (BossUI)
	{
		BossUI->UpdateHP(CurrentHP, MaxHP);
	}

	// 변신 중 AI를 정지시킵니다.
	AAIController* AICon = Cast<AAIController>(GetController());
	if (AICon && AICon->GetBrainComponent())
	{
		AICon->GetBrainComponent()->StopLogic(TEXT("Transformation"));
	}

	// 모든 클라이언트에 변신 몽타주와 HP 바 갱신을 전달합니다.
	Multicast_EnterRagePhase();

	// 서버에서도 몽타주를 직접 재생해야 종료 델리게이트가 정상 동작합니다.
	if (TransformationMontage)
	{
		PlayAnimMontage(TransformationMontage);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ABossMonster::EndRageTransformation);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, TransformationMontage);
		}
	}
	else
	{
		EndRageTransformation(nullptr, false);
	}
}

// 변신 몽타주 종료 후 무적 해제, HP 전체 회복, AI 재시작으로 2페이즈를 시작합니다.
void ABossMonster::EndRageTransformation(UAnimMontage* Montage, bool bInterrupted)
{
	bIsInvincible = false;
	bIsTargetable = true;
	CurrentHP = MaxHP;

	// 변신 완료 후 이름과 HP를 모든 클라이언트에 갱신합니다.
	Multicast_OnRageComplete(CurrentHP, MaxHP);

	AAIController* AICon = Cast<AAIController>(GetController());
	if (AICon && AICon->GetBrainComponent())
	{
		AICon->GetBrainComponent()->RestartLogic();
		AICon->GetBlackboardComponent()->SetValueAsBool(TEXT("bIsRagePhase"), true);
	}
}

// 무적 상태이면 데미지를 무시하고, 1페이즈에서 HP가 0에 도달하면 페이즈 전환을 시작합니다.
float ABossMonster::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// 무적 상태(변신 중)에는 데미지를 무시합니다.
	if (bIsInvincible) return 0.0f;

	// 1페이즈에서 HP가 0에 도달하면 사망 대신 페이즈 전환을 시작합니다.
	if (!bIsRagePhase && (CurrentHP - DamageAmount) <= 0.0f)
	{
		CurrentHP = 0.01f;
		Multicast_UpdateBossHP(0.0f, MaxHP);

		bIsTargetable = false;
		EnterRagePhase();
		return DamageAmount;
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Multicast_UpdateBossHP(CurrentHP, MaxHP);

	return ActualDamage;
}

// 현재 페이즈(1 or 2)에 맞는 공격 몽타주 목록에서 랜덤으로 재생합니다.
void ABossMonster::BaseAttack()
{
	if (!bCanAttack) return;

	TArray<UAnimMontage*>* TargetArray = bIsRagePhase ? &RageAttackMontages : &BaseAttackMontages;

	if (!TargetArray || TargetArray->Num() == 0) return;

	int32 RandomIndex = FMath::RandRange(0, TargetArray->Num() - 1);
	UAnimMontage* SelectedMontage = (*TargetArray)[RandomIndex];
	if (!SelectedMontage) return;

	bCanAttack = false;
	MulticastPlayAttackMontage(SelectedMontage);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ABossMonster::OnAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, SelectedMontage);
	}
}

// 공격 몽타주가 끝나면 공격 가능 플래그를 복구합니다.
void ABossMonster::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bCanAttack = true;
}

// 사망 시 보스 HP 바를 화면에서 제거한 뒤 부모(AEnemy)의 Die()를 호출합니다.
void ABossMonster::Die()
{
	if (BossUI)
	{
		BossUI->RemoveFromParent();
	}

	Super::Die();
}

// 회전 몽타주를 모든 클라이언트에서 재생합니다. 종료 델리게이트는 서버(BTTask)에서 따로 바인딩합니다.
void ABossMonster::Multicast_PlayTurnMontage_Implementation(UAnimMontage* Montage)
{
	if (!Montage) return;
	PlayAnimMontage(Montage);
}

// 점프 공격 몽타주를 지정 섹션부터 모든 클라이언트에서 재생합니다.
void ABossMonster::Multicast_PlayJumpAttackMontage_Implementation(UAnimMontage* Montage, FName SectionName)
{
	if (!Montage) return;
	PlayAnimMontage(Montage, 1.0f, SectionName);
}

// 모든 클라이언트에서 점프 공격 몽타주를 LandSmash 섹션으로 전환합니다.
void ABossMonster::Multicast_JumpToLandSmash_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && JumpAttackMontage)
	{
		AnimInstance->Montage_JumpToSection(FName("LandSmash"), JumpAttackMontage);
	}
}

// 모든 클라이언트에서 보스가 즉시 지정 방향을 향하도록 설정합니다.
void ABossMonster::Multicast_FaceBossToward_Implementation(FRotator NewRotation)
{
	SetActorRotation(NewRotation);
}

// 보스는 리시 시스템을 사용하지 않으므로 비어있습니다.
void ABossMonster::CheckLeash()
{
}

// 풀에서 재활성화될 때 보스 전투 상태(bIsEngaged, bIsRagePhase, HP 바)를 초기화합니다.
void ABossMonster::Revive()
{
	bIsEngaged   = false;
	bIsRagePhase = false;

	// 이동 속도를 1페이즈 기본값으로 복구합니다.
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = OriginalMoveSpeed;
	}

	// 잔여 HP 바가 있으면 제거합니다.
	if (BossUI)
	{
		BossUI->RemoveFromParent();
	}
}

// 점프 공격 시작: 도약 애니메이션(JumpStart 섹션)을 재생하고 물리 발사를 예약합니다.
void ABossMonster::ExecuteJumpAttack()
{
	bHasDamaged = false;
	if (!JumpAttackMontage) return;

	bIsJumpAttacking = true;
	bCanAttack       = false;

	// 모든 클라이언트에 JumpStart 섹션부터 몽타주를 동기화합니다.
	Multicast_PlayJumpAttackMontage(JumpAttackMontage, FName("JumpStart"));

	// 몽타주 종료 콜백은 서버에서만 바인딩합니다.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ABossMonster::OnAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, JumpAttackMontage);
	}
}

// 물리 발사: 플레이어 방향으로 보스를 LaunchCharacter하고 경고 데칼을 스폰합니다.
void ABossMonster::LaunchToAir()
{
	// 서버에서만 실행합니다. 클라이언트 AnimBP에서 호출되더라도 중복 실행을 방지합니다.
	if (!HasAuthority()) return;

	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	GetCharacterMovement()->GravityScale = 1.0f;

	// BB의 TargetPlayer를 우선 사용하고, 없으면 인덱스 0번으로 폴백합니다.
	ACharacter* Player = nullptr;
	AAIController* AIC = Cast<AAIController>(GetController());
	if (AIC && AIC->GetBlackboardComponent())
	{
		Player = Cast<ACharacter>(AIC->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetPlayer")));
	}
	if (!Player)
	{
		Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	}
	if (!Player) return;

	FVector BossLoc   = GetActorLocation();
	FVector PlayerLoc = Player->GetActorLocation();

	FVector ToPlayer = PlayerLoc - BossLoc;
	ToPlayer.Z = 0.f;
	float Distance = ToPlayer.Size();
	ToPlayer.Normalize();

	// 수평 힘: 거리에 비례, 수직 힘: 고정값으로 포물선 궤적을 만듭니다.
	FVector LaunchVelocity = (ToPlayer * Distance * 0.6f) + FVector(0.f, 0.f, 1200.f);

	// 모든 클라이언트에 즉시 방향을 동기화한 뒤 발사합니다.
	// SetActorRotation만 사용하면 복제 딜레이로 클라이언트가 틀린 방향을 보게 됩니다.
	Multicast_FaceBossToward(ToPlayer.Rotation());
	LaunchCharacter(LaunchVelocity, true, true);

	// 플레이어 발밑 지면에 경고 데칼을 모든 클라이언트에 스폰합니다.
	if (WarningDecalMaterial)
	{
		FVector TraceStart = FVector(PlayerLoc.X, PlayerLoc.Y, PlayerLoc.Z + 100.f);
		FVector TraceEnd   = FVector(PlayerLoc.X, PlayerLoc.Y, PlayerLoc.Z - 500.f);

		FHitResult GroundHit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		FVector DecalLocation = PlayerLoc;
		if (GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			DecalLocation = GroundHit.ImpactPoint;
		}

		// 히트 기준 위치를 캐싱합니다. 데칼이 제거된 후에도 AN_GroundSlam에서 사용됩니다.
		CachedImpactLocation = DecalLocation;
		Multicast_SpawnWarningDecal(DecalLocation);
	}
}

// 착지 시 점프 공격 중이었다면 LandSmash 섹션으로 강제 이동해 착지 타격 애니메이션을 재생합니다.
void ABossMonster::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (bIsJumpAttacking)
	{
		bIsJumpAttacking = false;
		GetCharacterMovement()->GravityScale = 1.0f;

		// 모든 클라이언트에 LandSmash 섹션 전환을 동기화합니다.
		// 경고 데칼 제거는 AN_GroundSlam AnimNotify 내부에서 처리합니다.
		Multicast_JumpToLandSmash();
	}
}
