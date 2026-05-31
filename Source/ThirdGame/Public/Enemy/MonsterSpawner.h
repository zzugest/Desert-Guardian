
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h" 
#include "MonsterSpawner.generated.h"

class USphereComponent;
class AEnemy;

USTRUCT(BlueprintType)
struct FMonsterSpawnInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EnemyRowName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpawnCount = 1;
};

UCLASS()
class THIRDGAME_API AMonsterSpawner : public AActor
{
    GENERATED_BODY()

public:
    AMonsterSpawner();

protected:
    virtual void BeginPlay() override;

    bool bIsPlayerInside = false;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    UDataTable* EnemyDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Data")
    TArray<FMonsterSpawnInfo> SpawnList;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn|Sensor")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn|Pool")
    TArray<AEnemy*> MonsterPool;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Pool")
    int32 MaxMonsterCount = 5;

    UFUNCTION()
    void OnSensorOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSensorOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void HandleMonsterDeath(AEnemy* DeadMonster);

    UFUNCTION()
    void RespawnMonster(AEnemy* MonsterToRespawn);

    bool bIsPlayerInZone = false;

    TArray<class AMyCharacter*> PlayersInZone;

    TMap<AEnemy*, FTimerHandle> RespawnTimerHandles;
};
