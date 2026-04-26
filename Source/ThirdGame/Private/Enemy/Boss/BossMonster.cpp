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

// 플레이어를 처음 감지하면 전투 시작 상태로 전환하고 보스 HP 바를 화면에 표시합니다.
void ABossMonster::OnBossTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !Actor->IsA<AMyCharacter>()) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		// 이미 전투 중이면 중복 AddToViewport를 방지합니다.
		if (bIsEngaged) return;

		bIsEngaged = true;

		if (BossUI)
		{
			BossUI->UpdateHP(CurrentHP, MaxHP);
			BossUI->UpdateBossName(Phase1Name);
			BossUI->AddToViewport();
		}
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

		// 착지 없이 전환됐으므로 Landed()가 호출되지 않아 경고 데칼을 여기서 제거합니다.
		if (WarningDecal)
		{
			WarningDecal->DestroyComponent();
			WarningDecal = nullptr;
		}
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

	// 변신 몽타주를 재생하고 종료 시 EndRageTransformation을 호출합니다.
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

	if (BossUI)
	{
		BossUI->UpdateHP(CurrentHP, MaxHP);
		BossUI->UpdateBossName(Phase2Name);
	}

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
		if (BossUI) BossUI->UpdateHP(CurrentHP, MaxHP);

		bIsTargetable = false;
		EnterRagePhase();
		return DamageAmount;
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (BossUI) BossUI->UpdateHP(CurrentHP, MaxHP);

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
	PlayAnimMontage(SelectedMontage);

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

	// JumpStart 섹션부터 몽타주를 재생합니다.
	PlayAnimMontage(JumpAttackMontage, 1.0f, FName("JumpStart"));

	// 몽타주 종료 시 bCanAttack을 복구합니다.
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
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	GetCharacterMovement()->GravityScale = 1.0f;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!Player) return;

	FVector BossLoc   = GetActorLocation();
	FVector PlayerLoc = Player->GetActorLocation();

	// 무기 Tip 소켓이 플레이어 발밑에 착지하도록 Root 목표 위치를 보정합니다.
	FVector AdjustedPlayerLoc = PlayerLoc;
	if (WeaponMesh)
	{
		FVector TipLoc    = WeaponMesh->GetSocketLocation(TEXT("Tip"));
		FVector TipOffset = FVector(TipLoc.X - BossLoc.X, TipLoc.Y - BossLoc.Y, 0.f);
		AdjustedPlayerLoc -= TipOffset;
	}

	FVector ToPlayer = AdjustedPlayerLoc - BossLoc;
	ToPlayer.Z = 0.f;
	float Distance = ToPlayer.Size();
	ToPlayer.Normalize();

	// 수평 힘: 거리에 비례, 수직 힘: 고정값으로 포물선 궤적을 만듭니다.
	FVector LaunchVelocity = (ToPlayer * Distance * 0.6f) + FVector(0.f, 0.f, 1200.f);

	SetActorRotation(ToPlayer.Rotation());
	LaunchCharacter(LaunchVelocity, true, true);

	// 플레이어 발밑 지면에 경고 데칼을 스폰합니다.
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

		WarningDecal = UGameplayStatics::SpawnDecalAtLocation(
			GetWorld(),
			WarningDecalMaterial,
			FVector(WarningDecalRadius),
			DecalLocation,
			FRotator(-90.f, 0.f, 0.f));
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

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && JumpAttackMontage)
		{
			AnimInstance->Montage_JumpToSection(FName("LandSmash"), JumpAttackMontage);
		}
	}
}
