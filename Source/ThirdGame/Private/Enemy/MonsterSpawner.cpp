// =========================================================================================
// MonsterSpawner.cpp
//
// [파일 역할]
// 스포너 영역에 배치되어 플레이어 접근 여부에 따라 오브젝트 풀(Pool)을 활성/비활성화하고,
// 처치된 몬스터를 일정 시간 뒤 재활성화시키는 스포너 클래스입니다.
//
// [버그 수정 내역]
// 1. RespawnTimerHandle 단일화 버그:
//    기존에 FTimerHandle 하나를 모든 몬스터가 공유했기 때문에, 5마리를 잡으면
//    SetTimer가 같은 핸들을 4번 덮어써 마지막 1마리만 재스폰됐습니다.
//    → TMap<AEnemy*, FTimerHandle>으로 몬스터마다 독립 핸들을 유지합니다.
//
// 2. 허수아비 현상:
//    HandleMonsterDeath에서 새 몬스터를 MonsterPool에 바로 추가했기 때문에,
//    플레이어가 재진입하면 OnSensorOverlapBegin이 리스폰 대기 중인 몬스터를
//    초기화 없이 그대로 노출했습니다.
//    → 새 몬스터는 RespawnMonster 완료 시점에만 MonsterPool에 추가합니다.
// =========================================================================================

#include "MonsterSpawner.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "EnemyHPBarWidget.h"
#include "ObjectPoolSubsystem.h"
#include "MinimapSubsystem.h"

// 플레이어 접근 감지용 SphereComponent를 설정하고 불필요한 Tick을 비활성화합니다.
AMonsterSpawner::AMonsterSpawner()
{
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    RootComponent   = DetectionSphere;

    DetectionSphere->SetSphereRadius(5000.0f);
    DetectionSphere->SetCollisionProfileName(TEXT("Trigger"));

    PrimaryActorTick.bCanEverTick = false;
}

// 오브젝트 풀에서 몬스터들을 미리 생성하고 비활성(대기) 상태로 MonsterPool에 보관합니다.
void AMonsterSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (DetectionSphere)
    {
        DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AMonsterSpawner::OnSensorOverlapBegin);
        DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AMonsterSpawner::OnSensorOverlapEnd);
    }

    if (!EnemyDataTable || SpawnList.IsEmpty()) return;

    for (const FMonsterSpawnInfo& SpawnInfo : SpawnList)
    {
        if (SpawnInfo.EnemyRowName.IsNone() || SpawnInfo.SpawnCount <= 0) continue;

        FEnemyData* Data = EnemyDataTable->FindRow<FEnemyData>(SpawnInfo.EnemyRowName, TEXT("SpawnerSetup"));
        if (!Data || !Data->EnemyClass) continue;

        for (int32 i = 0; i < SpawnInfo.SpawnCount; i++)
        {
            // 스포너 주변 랜덤 위치에 라인트레이스로 지면 좌표를 탐색합니다.
            FVector RandomOffset  = FVector(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), 0.0f);
            FVector TargetLocation = GetActorLocation() + RandomOffset;

            FVector StartTrace = TargetLocation + FVector(0.0f, 0.0f, 1000.0f);
            FVector EndTrace   = TargetLocation - FVector(0.0f, 0.0f, 2000.0f);

            FHitResult HitResult;
            FCollisionQueryParams CollisionParams;
            CollisionParams.AddIgnoredActor(this);

            bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, CollisionParams);

            if (bHit) TargetLocation = HitResult.ImpactPoint + FVector(0.0f, 0.0f, 100.0f);
            else      TargetLocation.Z = GetActorLocation().Z + 100.0f;

            FTransform SpawnTransform = FTransform(GetActorRotation(), TargetLocation);

            UObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
            AEnemy* SpawnedMonster = PoolSubsystem ? Cast<AEnemy>(PoolSubsystem->GetPooledActor(Data->EnemyClass)) : nullptr;

            if (SpawnedMonster)
            {
                SpawnedMonster->SetActorLocation(TargetLocation);
                SpawnedMonster->EnemyDataTable = EnemyDataTable;
                SpawnedMonster->EnemyRowName   = SpawnInfo.EnemyRowName;
                SpawnedMonster->MySpawner      = this;
                SpawnedMonster->Revive();
            }
            else
            {
                SpawnTransform = FTransform(GetActorRotation(), TargetLocation);
                SpawnedMonster = GetWorld()->SpawnActorDeferred<AEnemy>(
                    Data->EnemyClass, SpawnTransform, nullptr, nullptr,
                    ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

                if (SpawnedMonster)
                {
                    SpawnedMonster->EnemyDataTable = EnemyDataTable;
                    SpawnedMonster->EnemyRowName   = SpawnInfo.EnemyRowName;
                    SpawnedMonster->MySpawner      = this;
                    UGameplayStatics::FinishSpawningActor(SpawnedMonster, SpawnTransform);
                }
            }

            if (!SpawnedMonster) continue;

            SpawnedMonster->OnMonsterDied.RemoveDynamic(this, &AMonsterSpawner::HandleMonsterDeath);
            SpawnedMonster->OnMonsterDied.AddDynamic(this, &AMonsterSpawner::HandleMonsterDeath);

            // 처음에는 비활성 상태로 대기합니다.
            SpawnedMonster->SetActorHiddenInGame(true);
            SpawnedMonster->SetActorEnableCollision(false);
            SpawnedMonster->SetActorTickEnabled(false);
            SpawnedMonster->GetCharacterMovement()->SetMovementMode(MOVE_None);

            MonsterPool.Add(SpawnedMonster);
        }
    }
}

// 플레이어가 스포너 구역에 진입하면 MonsterPool의 몬스터들을 활성화합니다.
// MonsterPool에는 활성 몬스터만 있으므로 리스폰 대기 중인 몬스터는 노출되지 않습니다.
void AMonsterSpawner::OnSensorOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor != UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)) return;

    bIsPlayerInZone = true;

    for (AEnemy* Monster : MonsterPool)
    {
        if (!Monster) continue;

        Monster->SetActorHiddenInGame(false);
        Monster->SetActorEnableCollision(true);
        Monster->SetActorTickEnabled(true);
        Monster->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

        // AEnemy::AllActiveEnemies.Add(Monster); // [비활성화] Overlap Sphere로 대체

        if (UGameInstance* GI = GetGameInstance())
        {
            if (UMinimapSubsystem* MinimapSys = GI->GetSubsystem<UMinimapSubsystem>())
            {
                MinimapSys->RegisterMarker(Monster, EMinimapMarkerType::Enemy);
            }
        }
    }
}

// 플레이어가 구역을 벗어나면 살아있는 몬스터들을 비활성(슬립) 상태로 전환합니다.
void AMonsterSpawner::OnSensorOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor != UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)) return;

    bIsPlayerInZone = false;

    for (AEnemy* Monster : MonsterPool)
    {
        if (!Monster || Monster->bIsDead) continue;

        Monster->SetActorHiddenInGame(true);
        Monster->SetActorEnableCollision(false);
        Monster->SetActorTickEnabled(false);

        // AEnemy::AllActiveEnemies.Remove(Monster); // [비활성화] Overlap Sphere로 대체

        if (UGameInstance* GI = GetGameInstance())
        {
            if (UMinimapSubsystem* MinimapSys = GI->GetSubsystem<UMinimapSubsystem>())
            {
                MinimapSys->UnregisterMarker(Monster);
            }
        }

        if (Monster->GetCharacterMovement())
        {
            Monster->GetCharacterMovement()->SetMovementMode(MOVE_None);
            Monster->GetCharacterMovement()->MaxWalkSpeed = Monster->OriginalMoveSpeed;
        }

        Monster->bIsReturning = false;

        AAIController* AICon = Cast<AAIController>(Monster->GetController());
        if (AICon && AICon->GetBlackboardComponent())
        {
            AICon->GetBlackboardComponent()->ClearValue(TEXT("TargetPlayer"));
            AICon->GetBlackboardComponent()->SetValueAsBool(TEXT("IsReturning"), false);
        }

        Monster->CurrentHP = Monster->MaxHP;

        if (Monster->HPBarWidget)
        {
            UEnemyHPBarWidget* HPWidget = Cast<UEnemyHPBarWidget>(Monster->HPBarWidget->GetUserWidgetObject());
            if (HPWidget) HPWidget->UpdateHPWidget(Monster->CurrentHP, Monster->MaxHP);
        }

        Monster->SetActorLocation(Monster->HomeLocation);
    }
}

// 몬스터가 사망하면 MonsterPool에서 제거하고 개별 리스폰 타이머를 예약합니다.
void AMonsterSpawner::HandleMonsterDeath(AEnemy* DeadMonster)
{
    if (!DeadMonster) return;

    // 죽은 몬스터를 풀·미니맵에서 제거하고 델리게이트를 해제합니다.
    MonsterPool.Remove(DeadMonster);
    // AEnemy::AllActiveEnemies.Remove(DeadMonster); // [비활성화] Overlap Sphere로 대체
    DeadMonster->OnMonsterDied.RemoveDynamic(this, &AMonsterSpawner::HandleMonsterDeath);

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UMinimapSubsystem* MinimapSys = GI->GetSubsystem<UMinimapSubsystem>())
        {
            MinimapSys->UnregisterMarker(DeadMonster);
        }
    }

    FName RowNameToRespawn = DeadMonster->EnemyRowName;

    UObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
    if (!PoolSubsystem) return;

    FEnemyData* Data = EnemyDataTable->FindRow<FEnemyData>(RowNameToRespawn, TEXT("RespawnSetup"));
    if (!Data || !Data->EnemyClass) return;

    // 풀에서 새 몬스터를 가져와 데이터만 초기화합니다.
    AEnemy* NewMonster = Cast<AEnemy>(PoolSubsystem->GetPooledActor(Data->EnemyClass));
    if (!NewMonster) return;

    NewMonster->EnemyDataTable = EnemyDataTable;
    NewMonster->EnemyRowName   = RowNameToRespawn;
    NewMonster->MySpawner      = this;
    NewMonster->OnMonsterDied.AddDynamic(this, &AMonsterSpawner::HandleMonsterDeath);

    // 리스폰 대기 중에는 완전히 숨겨둡니다.
    // ★ MonsterPool에 추가하지 않아 플레이어 재진입 시 허수아비처럼 노출되지 않습니다.
    NewMonster->SetActorHiddenInGame(true);
    NewMonster->SetActorEnableCollision(false);
    NewMonster->SetActorTickEnabled(false);

    // 몬스터마다 독립된 타이머 핸들을 사용합니다.
    // 단일 FTimerHandle을 공유하면 SetTimer 호출마다 이전 타이머가 덮어써져
    // 마지막 1마리 타이머만 살아남는 버그가 발생합니다.
    FTimerHandle& Handle = RespawnTimerHandles.FindOrAdd(NewMonster);
    FTimerDelegate RespawnDelegate;
    RespawnDelegate.BindUFunction(this, FName("RespawnMonster"), NewMonster);
    GetWorld()->GetTimerManager().SetTimer(Handle, RespawnDelegate, 10.0f, false);
}

// 타이머 만료 후 몬스터를 Revive하고 MonsterPool에 정식으로 추가합니다.
void AMonsterSpawner::RespawnMonster(AEnemy* MonsterToRespawn)
{
    if (!MonsterToRespawn) return;

    // 사용 완료된 타이머 핸들을 맵에서 제거합니다.
    RespawnTimerHandles.Remove(MonsterToRespawn);

    // 스포너 주변 랜덤 지면 위치를 탐색합니다.
    FVector RandomOffset   = FVector(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), 0.0f);
    FVector TargetLocation = GetActorLocation() + RandomOffset;

    FVector StartTrace = TargetLocation + FVector(0.0f, 0.0f, 1000.0f);
    FVector EndTrace   = TargetLocation - FVector(0.0f, 0.0f, 2000.0f);

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);
    CollisionParams.AddIgnoredActor(MonsterToRespawn);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, CollisionParams);
    if (bHit) TargetLocation = HitResult.ImpactPoint + FVector(0.0f, 0.0f, 80.0f);

    MonsterToRespawn->SetActorLocation(TargetLocation);
    MonsterToRespawn->Revive();

    // 리스폰 완료 시점에 MonsterPool에 정식 추가합니다.
    MonsterPool.Add(MonsterToRespawn);

    // 플레이어가 구역 안에 있으면 즉시 활성화, 밖이면 숨겨둡니다.
    if (bIsPlayerInZone)
    {
        MonsterToRespawn->SetActorHiddenInGame(false);
        MonsterToRespawn->SetActorEnableCollision(true);
        MonsterToRespawn->SetActorTickEnabled(true);
        MonsterToRespawn->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

        // AEnemy::AllActiveEnemies.Add(MonsterToRespawn); // [비활성화] Overlap Sphere로 대체

        if (UGameInstance* GI = GetGameInstance())
        {
            if (UMinimapSubsystem* MinimapSys = GI->GetSubsystem<UMinimapSubsystem>())
            {
                MinimapSys->RegisterMarker(MonsterToRespawn, EMinimapMarkerType::Enemy);
            }
        }
    }
    else
    {
        MonsterToRespawn->SetActorHiddenInGame(true);
        MonsterToRespawn->SetActorEnableCollision(false);
        MonsterToRespawn->SetActorTickEnabled(false);
        MonsterToRespawn->GetCharacterMovement()->SetMovementMode(MOVE_None);
    }
}
