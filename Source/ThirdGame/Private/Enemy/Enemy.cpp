// =========================================================================================
// Enemy.cpp
//
// [파일 역할]
// 게임 내 모든 적(Enemy) 캐릭터의 공통 기반 클래스입니다.
// DataTable(FEnemyData) 기반으로 스탯/AI/애니메이션을 초기화하고,
// AI 퍼셉션(시야) 감지, 리시 시스템(Leash), 오브젝트 풀 연동을 담당합니다.
//
// [AllActiveEnemies 정적 목록]
// 타겟팅 시스템(TargetingComponent)과 미니맵(MinimapWidget)이
// 매 프레임 GetAllActorsOfClass 없이 적 목록을 참조할 수 있도록
// 스폰 시 등록, 사망/풀 반환 시 제거되는 약참조(TWeakObjectPtr) 배열입니다.
// =========================================================================================

#include "Enemy.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "EnemyHPBarWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISense_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "CoreMinimal.h"
#include "MyCharacter.h"
#include "NavigationInvokerComponent.h"
#include "BrainComponent.h"
#include "MonsterSpawner.h"
#include "NPC/Quest/QuestSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "ObjectPoolSubsystem.h"
#include "MinimapSubsystem.h"

// [비활성화] 타겟팅 → Overlap Sphere / 미니맵 → MinimapSubsystem으로 대체
// TArray<TWeakObjectPtr<AEnemy>> AEnemy::AllActiveEnemies;

// 캡슐·스켈레탈 메시·HP 바·AI 퍼셉션·내비게이션 인보커·타겟 마커의 기본값을 초기화합니다.
AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -96.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel5, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	MaxHP = 100.0f;

	// 항상 카메라를 향하도록 절대(Absolute) 위치·회전 모드로 HP 바를 생성합니다.
	HPBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidget"));
	HPBarWidget->SetupAttachment(GetCapsuleComponent());
	HPBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
	HPBarWidget->SetWidgetSpace(EWidgetSpace::World);
	HPBarWidget->SetDrawSize(FVector2D(150.0f, 100.0f));
	HPBarWidget->SetUsingAbsoluteRotation(true);
	HPBarWidget->SetUsingAbsoluteScale(true);

	// 플레이어·적을 시야(Sight)로 감지하는 AI 퍼셉션 컴포넌트를 설정합니다.
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SightConfig      = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	if (SightConfig)
	{
		SightConfig->SightRadius                              = 1500.0f;
		SightConfig->LoseSightRadius                          = 3000.0f;
		SightConfig->PeripheralVisionAngleDegrees             = 180.0f;
		SightConfig->SetMaxAge(5.0f);
		SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

		AIPerceptionComp->ConfigureSense(*SightConfig);
		AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
	}

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 넓은 오픈 월드에서 적 주변 일부 구역만 내비게이션을 생성해 메모리를 절약합니다.
	NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
	NavInvoker->SetGenerationRadii(1500.0f, 2000.0f);


	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 타겟팅 시 머리 위에 표시되는 마커 메시를 생성합니다.
	TargetMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMarkerMesh"));
	TargetMarkerMesh->SetupAttachment(GetRootComponent());
	TargetMarkerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	TargetMarkerMesh->SetRelativeRotation(FRotator(180.0f, 0.0f, 0.0f));
	TargetMarkerMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));
	TargetMarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TargetMarkerMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MarkerMeshAsset(TEXT("/Script/Engine.StaticMesh'/Engine/VREditor/BasicMeshes/SM_Pyramid_01.SM_Pyramid_01'"));
	if (MarkerMeshAsset.Succeeded())
	{
		TargetMarkerMesh->SetStaticMesh(MarkerMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MarkerMaterialAsset(TEXT("/Script/Engine.Material'/Engine/EngineDebugMaterials/VertexColorViewMode_RedOnly.VertexColorViewMode_RedOnly'"));
	if (MarkerMaterialAsset.Succeeded())
	{
		TargetMarkerMesh->SetMaterial(0, MarkerMaterialAsset.Object);
	}
}

// DataTable에서 스탯·AI·애니메이션을 로드하고, AI 시작·리시 타이머·전역 목록 등록을 수행합니다.
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (!EnemyDataTable || EnemyRowName.IsNone()) return;

	FEnemyData* Data = EnemyDataTable->FindRow<FEnemyData>(EnemyRowName, TEXT("EnemySetup"));
	if (!Data) return;

	MaxHP            = Data->MaxHP;
	AttackPower      = Data->AttackPower;
	AttackRange      = Data->AttackRange;
	AttackCooldown   = Data->AttackCooldown;
	BaseAttackDamage = Data->BaseAttackDamage;
	BaseAttackMontages = Data->BaseAttackMontages;
	MyDisplayName    = Data->EnemyDisplayName;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Data->MoveSpeed;
		OriginalMoveSpeed = Data->MoveSpeed;
		GetCharacterMovement()->bUseRVOAvoidance = true;
		GetCharacterMovement()->AvoidanceWeight  = 1.0f;
	}

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCapsuleSize(Data->CapsuleRadius, Data->CapsuleHalfHeight);
	}

	if (Data->AnimClass) GetMesh()->SetAnimInstanceClass(Data->AnimClass);

	// 데디케이티드 서버는 렌더링이 없어 본 트랜스폼 계산을 생략하는데,
	// 무기 소켓 위치 기반 공격 트레이스가 서버에서 정확히 동작하려면 항상 계산해야 합니다.
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// 블루프린트에서 추가된 스태틱 메시 컴포넌트 중 이름에 "Weapon"이 포함된 것을 무기로 캐싱합니다.
	// NormalMonster·EliteMonster 등 모든 서브클래스가 공통으로 사용합니다.
	TArray<UStaticMeshComponent*> StaticMeshes;
	GetComponents<UStaticMeshComponent>(StaticMeshes);
	for (UStaticMeshComponent* SMComp : StaticMeshes)
	{
		if (!SMComp) continue;
		if (SMComp->GetName().Contains(TEXT("Weapon")))
		{
			WeaponMesh = SMComp;
			break;
		}
	}

	// AI 퍼셉션 이벤트를 연결해 플레이어 감지·소실 시 블랙보드가 갱신되도록 합니다.
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemy::OnTargetDetected);
	}

	AAIController* AIC = Cast<AAIController>(GetController());
	if (AIC && Data->BehaviorTree)
	{
		AIC->RunBehaviorTree(Data->BehaviorTree);
	}

	if (HPBarWidget)
	{
		HPBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, Data->HPBarHeight));
	}

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnOverlapBegin);

	CurrentHP = MaxHP;

	if (HPBarWidget)
	{
		UEnemyHPBarWidget* EnemyWidget = Cast<UEnemyHPBarWidget>(HPBarWidget->GetUserWidgetObject());
		if (EnemyWidget)
		{
			EnemyWidget->UpdateHPWidget(CurrentHP, MaxHP);

			if (EnemyWidget->TXT_EnemyName)
			{
				EnemyWidget->TXT_EnemyName->SetText(MyDisplayName);
				EnemyWidget->TXT_EnemyName->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	// 스폰 위치를 기억하고 0.5초마다 리시 범위 초과를 감지합니다.
	HomeLocation = GetActorLocation();
	GetWorld()->GetTimerManager().SetTimer(LeashTimerHandle, this, &AEnemy::CheckLeash, 0.5f, true);

	// 스포너가 없는 적(보스 등 직접 배치)만 여기서 미니맵에 등록합니다.
	// 스포너가 있는 적은 OnSensorOverlapBegin에서 등록합니다.
	if (!MySpawner)
	{
		// AllActiveEnemies.Add(this); // [비활성화] Overlap Sphere로 대체

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMinimapSubsystem* MinimapSys = GI->GetSubsystem<UMinimapSubsystem>())
			{
				MinimapSys->RegisterMarker(this, EMinimapMarkerType::Enemy);
			}
		}
	}

	// HP 바 빌보딩에 필요한 카메라 매니저를 한 번만 조회해 캐싱합니다.
	CachedCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
}

// 레벨 전환·종료 시 타이머, 델리게이트, 전역 목록을 정리합니다.
void AEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(DeathTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(LeashTimerHandle);

	// 레벨 전환 시 stale 콜백 방지를 위해 델리게이트를 해제합니다.
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(this, &AEnemy::OnTargetDetected);
	}

	// 스포너가 없는 적(보스 등 직접 배치)만 여기서 미니맵에서 제거합니다.
	if (!MySpawner)
	{
		// AllActiveEnemies.Remove(this); // [비활성화] Overlap Sphere로 대체

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMinimapSubsystem* MinimapSys = GI->GetSubsystem<UMinimapSubsystem>())
			{
				MinimapSys->UnregisterMarker(this);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

// HP 바 위젯이 항상 카메라를 정면으로 바라보도록 매 프레임 Yaw 회전을 갱신합니다.
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HPBarWidget || !CachedCameraManager) return;

	// FindLookAtRotation은 두 지점 사이의 정확한 방향을 반환하므로
	// 어느 각도에서 봐도 HP 바가 정면을 유지합니다.
	FVector WidgetLoc = HPBarWidget->GetComponentLocation();
	FVector CameraLoc = CachedCameraManager->GetCameraLocation();
	FRotator LookAt   = UKismetMathLibrary::FindLookAtRotation(WidgetLoc, CameraLoc);

	// Pitch·Roll은 0으로 고정해 HP 바가 수평을 유지하며 Yaw만 회전합니다.
	HPBarWidget->SetWorldRotation(FRotator(0.f, LookAt.Yaw, 0.f));
}

// 현재 미사용입니다.
void AEnemy::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

// 데미지 이벤트를 받아 HP를 갱신하고, 0 이하가 되면 사망 처리를 시작합니다.
float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 이미 사망 처리된 적에게는 데미지를 무시합니다.
	if (bIsDead) return 0.0f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.0f) return ActualDamage;

	CurrentHP -= ActualDamage;

	if (CurrentHP <= 0.0f && !bIsDead)
	{
		bIsDead       = true;
		bIsTargetable = false;

		// 이동·AI를 멈추고 사망 몽타주를 재생합니다.
		GetCharacterMovement()->SetMovementMode(MOVE_None);
		StopAnimMontage();

		AAIController* AICon = Cast<AAIController>(GetController());
		if (AICon && AICon->GetBrainComponent())
		{
			AICon->GetBrainComponent()->StopLogic(TEXT("Dead"));
		}

		if (DeathMontage)
		{
			float AnimDuration = PlayAnimMontage(DeathMontage);
			// DeathTimerHandle은 멤버 변수로, Revive() 시 ClearTimer로 취소 가능합니다.
			GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, this, &AEnemy::Die, AnimDuration, false);
		}
		else
		{
			Die();
		}
	}

	if (HPBarWidget)
	{
		UEnemyHPBarWidget* EnemyWidget = Cast<UEnemyHPBarWidget>(HPBarWidget->GetUserWidgetObject());
		if (EnemyWidget) EnemyWidget->UpdateHPWidget(CurrentHP, MaxHP);
	}

	return ActualDamage;
}

// 애니메이션 타이밍에 맞춰 실제 데미지를 적용하는 함수입니다 (하위 클래스에서 오버라이드).
void AEnemy::ExecuteAttackHit()
{
}

// 등록된 공격 몽타주 목록 중 하나를 랜덤으로 재생하고 쿨다운 타이머를 시작합니다.
void AEnemy::BaseAttack()
{
	if (!bCanAttack || BaseAttackMontages.Num() == 0) return;

	int32 RandomIndex          = FMath::RandRange(0, BaseAttackMontages.Num() - 1);
	UAnimMontage* SelectedMontage = BaseAttackMontages[RandomIndex];
	if (!SelectedMontage) return;

	MulticastPlayAttackMontage(SelectedMontage);

	bCanAttack = false;
	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AEnemy::ResetAttack, AttackCooldown, false);
}

// 서버에서 선택한 공격 몽타주를 모든 클라이언트 화면에서 재생합니다.
void AEnemy::MulticastPlayAttackMontage_Implementation(UAnimMontage* Montage)
{
	if (Montage) PlayAnimMontage(Montage);
}

// 모든 클라이언트에서 각자의 QuestSubsystem에 처치 진행도를 반영합니다.
void AEnemy::Multicast_UpdateQuestObjective_Implementation(EQuestTaskType TaskType, FName TargetRowName)
{
	UGameInstance* GameInst = GetWorld()->GetGameInstance();
	if (!GameInst) return;

	UQuestSubsystem* QuestSys = GameInst->GetSubsystem<UQuestSubsystem>();
	if (QuestSys)
	{
		QuestSys->UpdateQuestObjective(TaskType, TargetRowName, 1);
	}
}

// 공격 쿨다운이 끝나 다음 공격이 가능하도록 플래그를 복구합니다.
void AEnemy::ResetAttack()
{
	bCanAttack = true;
}

// AI 시야 자극 이벤트를 받아 플레이어 감지·소실에 따라 블랙보드의 타겟을 갱신합니다.
void AEnemy::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	AMyCharacter* PlayerCharacter = Cast<AMyCharacter>(Actor);
	if (!PlayerCharacter) return;

	// 사망한 플레이어는 감지하지 않습니다.
	if (PlayerCharacter->HasStateTag("State.Dead")) return;

	if (GetName().Contains(TEXT("NormalOrc_1_C_0")))
	{
		UE_LOG(LogTemp, Warning, TEXT("[DiagPerception] 플레이어 %s: %s"),
			Stimulus.WasSuccessfullySensed() ? TEXT("감지") : TEXT("소실"),
			*GetActorLocation().ToString());
	}

	AAIController* AIC = Cast<AAIController>(GetController());
	if (!AIC) return;

	UBlackboardComponent* BBComp = AIC->GetBlackboardComponent();
	if (!BBComp) return;

	AMyCharacter* CurrentTarget = Cast<AMyCharacter>(BBComp->GetValueAsObject(TEXT("TargetPlayer")));

	if (Stimulus.WasSuccessfullySensed())
	{
		// 이미 추적 중인 타겟이 있으면 교체하지 않습니다.
		if (CurrentTarget) return;

		BBComp->SetValueAsObject(TEXT("TargetPlayer"), PlayerCharacter);
	}
	else
	{
		// 사라진 액터가 현재 타겟일 때만 클리어합니다.
		if (CurrentTarget != PlayerCharacter) return;

		BBComp->ClearValue(TEXT("TargetPlayer"));
	}
}

// 퀘스트 진행 업데이트 후 오브젝트 풀로 반환하고 OnMonsterDied 델리게이트를 브로드캐스트합니다.
void AEnemy::Die()
{
	// 모든 클라이언트의 QuestSubsystem에 처치 사실을 알립니다.
	Multicast_UpdateQuestObjective(EQuestTaskType::Hunt, EnemyRowName);

	// 스포너가 없는 적(보스 등 직접 배치)만 여기서 미니맵에서 제거합니다.
	// 스포너가 있는 적은 HandleMonsterDeath에서 제거합니다.
	if (!MySpawner)
	{
		// AllActiveEnemies.Remove(this); // [비활성화] Overlap Sphere로 대체

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMinimapSubsystem* MinimapSys = GI->GetSubsystem<UMinimapSubsystem>())
			{
				MinimapSys->UnregisterMarker(this);
			}
		}
	}

	UObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (PoolSubsystem)
	{
		PoolSubsystem->ReturnActorToPool(this);
	}

	OnMonsterDied.Broadcast(this);
}

// 풀에서 재활성화될 때 체력·이동·AI를 완전히 초기화하고 전역 목록에 다시 등록합니다.
void AEnemy::Revive()
{
	// 사망 타이머가 대기 중이면 취소합니다
	// (몽타주 종료 전에 Revive되면 Die()가 살아있는 적에게 호출되는 버그 방지).
	GetWorld()->GetTimerManager().ClearTimer(DeathTimerHandle);

	// DataTable에서 최신 스탯을 다시 읽어 0 이하 체력 문제를 방지합니다.
	if (EnemyDataTable && !EnemyRowName.IsNone())
	{
		FEnemyData* Data = EnemyDataTable->FindRow<FEnemyData>(EnemyRowName, TEXT("ReviveSetup"));
		if (Data)
		{
			MaxHP             = Data->MaxHP;
			OriginalMoveSpeed = Data->MoveSpeed;
		}
	}

	CurrentHP     = MaxHP;
	bIsDead       = false;
	bIsTargetable = true;

	if (HPBarWidget)
	{
		UEnemyHPBarWidget* EnemyWidget = Cast<UEnemyHPBarWidget>(HPBarWidget->GetUserWidgetObject());
		if (EnemyWidget) EnemyWidget->UpdateHPWidget(CurrentHP, MaxHP);
	}

	// 숨김 해제 및 충돌 복구를 합니다.
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		GetCharacterMovement()->MaxWalkSpeed = OriginalMoveSpeed;
	}

	// AI 로직을 재시작합니다.
	AAIController* AICon = Cast<AAIController>(GetController());
	if (AICon && AICon->GetBrainComponent())
	{
		AICon->GetBrainComponent()->RestartLogic();
	}

	HomeLocation  = GetActorLocation();
	bIsReturning  = false;

	GetWorld()->GetTimerManager().SetTimer(LeashTimerHandle, this, &AEnemy::CheckLeash, 0.5f, true);

	// 스포너가 없는 적(보스 등 직접 배치)만 여기서 미니맵에 다시 등록합니다.
	// 스포너가 있는 적은 RespawnMonster에서 등록합니다.
	if (!MySpawner)
	{
		// AllActiveEnemies.RemoveAll([](const TWeakObjectPtr<AEnemy>& W) { return !W.IsValid(); }); // [비활성화]
		// AllActiveEnemies.Add(this); // [비활성화] Overlap Sphere로 대체

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMinimapSubsystem* MinimapSys = GI->GetSubsystem<UMinimapSubsystem>())
			{
				MinimapSys->RegisterMarker(this, EMinimapMarkerType::Enemy);
			}
		}
	}
}

// 0.5초마다 스폰 위치(HomeLocation)와의 거리를 확인해 리시 범위 초과 시 귀환을 시작합니다.
void AEnemy::CheckLeash()
{
	if (bIsDead || IsHidden()) return;

	AAIController* AICon = Cast<AAIController>(GetController());
	float DistanceFromHome = FVector::Dist(GetActorLocation(), HomeLocation);

	if (!bIsReturning && DistanceFromHome > LeashRadius)
	{
		// 리시 범위를 벗어나면 HP를 회복하고 귀환 모드로 전환합니다.
		UE_LOG(LogTemp, Warning, TEXT("[Leash][%s] 귀환 시작 - 거리: %.1f"), *GetName(), DistanceFromHome);
		bIsReturning = true;
		CurrentHP    = MaxHP;

		if (HPBarWidget)
		{
			UEnemyHPBarWidget* EnemyWidget = Cast<UEnemyHPBarWidget>(HPBarWidget->GetUserWidgetObject());
			if (EnemyWidget) EnemyWidget->UpdateHPWidget(CurrentHP, MaxHP);
		}

		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = 1000.0f;
		}

		if (AICon && AICon->GetBlackboardComponent())
		{
			AICon->GetBlackboardComponent()->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
			AICon->GetBlackboardComponent()->SetValueAsBool(TEXT("IsReturning"), true);
		}
	}
	else if (bIsReturning && DistanceFromHome <= 500.0f)
	{
		// 귀환 완료: 이동 속도와 플래그를 복구합니다.
		bIsReturning = false;

		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = OriginalMoveSpeed;
		}

		if (AICon && AICon->GetBlackboardComponent())
		{
			AICon->GetBlackboardComponent()->SetValueAsBool(TEXT("IsReturning"), false);
		}

		// 귀환 완료 시 이전 타겟을 해제하고 퍼셉션을 초기화합니다.
		// ForgetAll()로 퍼셉션 기억을 지워야 시야 내 플레이어를 처음 본 것처럼 재감지합니다.
		// (블랙보드만 지우면 퍼셉션 상태 변화가 없어 OnTargetDetected가 재발동되지 않습니다.)
		AAIController* AICon2 = Cast<AAIController>(GetController());
		if (AICon2 && AICon2->GetBlackboardComponent())
		{
			AICon2->GetBlackboardComponent()->ClearValue(TEXT("TargetPlayer"));
		}
		if (AIPerceptionComp)
		{
			AIPerceptionComp->ForgetAll();
		}

		// 플레이어가 구역을 벗어난 상태라면 슬립 모드로 전환합니다.
		if (MySpawner && !MySpawner->bIsPlayerInZone)
		{
			SetActorHiddenInGame(true);
			SetActorEnableCollision(false);
			SetActorTickEnabled(false);

			if (GetCharacterMovement())
			{
				GetCharacterMovement()->SetMovementMode(MOVE_None);
			}
		}
	}
}

// 이 몬스터를 특정 플레이어에게 복제할지 결정합니다.
// 스포너 구역 안에 있는 플레이어에게만 복제하여 구역 밖 클라이언트에게는 보이지 않도록 합니다.
bool AEnemy::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	if (MySpawner)
	{
		const AMyCharacter* PlayerChar = Cast<AMyCharacter>(ViewTarget);
		if (PlayerChar)
		{
			return MySpawner->PlayersInZone.Contains(const_cast<AMyCharacter*>(PlayerChar));
		}
	}

	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

// 서버에서 변경된 CurrentHP와 bIsDead를 클라이언트에 복제합니다.
void AEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemy, CurrentHP);
	DOREPLIFETIME(AEnemy, bIsDead);
	DOREPLIFETIME(AEnemy, EnemyDataTable);
	DOREPLIFETIME(AEnemy, EnemyRowName);
}

// 클라이언트에서 CurrentHP가 갱신되면 HP 바 위젯을 업데이트합니다.
void AEnemy::OnRep_CurrentHP()
{
	if (!HPBarWidget) return;

	UEnemyHPBarWidget* EnemyWidget = Cast<UEnemyHPBarWidget>(HPBarWidget->GetUserWidgetObject());
	if (EnemyWidget) EnemyWidget->UpdateHPWidget(CurrentHP, MaxHP);
}

// 클라이언트에서 bIsDead가 true로 변경되면 사망 처리를 수행합니다.
void AEnemy::OnRep_bIsDead()
{
	if (!bIsDead) return;

	bIsTargetable = false;

	GetCharacterMovement()->SetMovementMode(MOVE_None);
	StopAnimMontage();

	// 클라이언트에는 AIController가 없으므로 null 체크 후 처리합니다.
	AAIController* AICon = Cast<AAIController>(GetController());
	if (AICon && AICon->GetBrainComponent())
	{
		AICon->GetBrainComponent()->StopLogic(TEXT("Dead"));
	}

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
}

// EnemyDataTable 또는 EnemyRowName이 복제되면 클라이언트에서 AnimClass를 설정합니다.
void AEnemy::OnRep_EnemySetup()
{
	if (!EnemyDataTable || EnemyRowName.IsNone()) return;

	FEnemyData* Data = EnemyDataTable->FindRow<FEnemyData>(EnemyRowName, TEXT("OnRep_EnemySetup"));
	if (!Data) return;

	if (Data->AnimClass) GetMesh()->SetAnimInstanceClass(Data->AnimClass);
}

// 타겟팅 마커 메시와 이름 텍스트의 가시성을 동시에 전환합니다.
void AEnemy::SetTargetMarkerVisibility(bool bShow)
{
	if (!TargetMarkerMesh) return;

	TargetMarkerMesh->SetVisibility(bShow);

	if (HPBarWidget)
	{
		UEnemyHPBarWidget* EnemyWidget = Cast<UEnemyHPBarWidget>(HPBarWidget->GetUserWidgetObject());
		if (EnemyWidget && EnemyWidget->TXT_EnemyName)
		{
			ESlateVisibility TextVisibility = bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
			EnemyWidget->TXT_EnemyName->SetVisibility(TextVisibility);
		}
	}
}
