// =========================================================================================
// MonsterSpawner.h
//
// [���� ���]
// �� Ư�� ������ ��ġ�Ǿ� �÷��̾� ���� ���ο� ���� ���� Ǯ(Pool)�� Ȱ��/��Ȱ��ȭ�ϰ�, óġ�� ���͸� ���� �ð� �� ��Ȱ��Ű�� ������ ���� Ŭ���� ����Դϴ�.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h" 
#include "MonsterSpawner.generated.h"

class USphereComponent;
class AEnemy;

// -----------------------------------------------------------------------------
// FMonsterSpawnInfo
// �����ʿ� ����� ���� ���� ��Ʈ�� ����(���� �� ����)�� ��� ������ ����ü�Դϴ�.
// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FMonsterSpawnInfo
{
    GENERATED_BODY()

    // ��ȯ�� ������ ������ ���̺� ��(Row) �̸��Դϴ�. (��: NormalOrc_1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EnemyRowName;

    // �ش� ���� ���� ������ �������Դϴ�.
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
    // ������ �ε� �� ���� Ǯ�� ���� �����ϰ� �÷��̾� ���� ������ �մϴ�.
    virtual void BeginPlay() override;

    // ���������� �÷��̾ ���� ������ ������ �������� �Ǻ��ϴ� �÷����Դϴ�.
    bool bIsPlayerInside = false;

public:
    // ������ ���͵��� ��ü���� ������ ��ϵ� ������ ������ ���̺��Դϴ�.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    UDataTable* EnemyDataTable;

    // ���� ������ ��ġ�� ��ȯ�� ���� ���� ��Ʈ ����Ʈ�Դϴ�.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Data")
    TArray<FMonsterSpawnInfo> SpawnList;

    // �÷��̾��� ������ �����ϱ� ���� �ݰ� ���� �浹ü ������Ʈ�Դϴ�.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn|Sensor")
    USphereComponent* DetectionSphere;

    // �����ص� �� �÷��̾� �Ÿ��� ���� On/Off ��ų ���� ��ü���� �ӽ� ���� �迭�Դϴ�.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn|Pool")
    TArray<AEnemy*> MonsterPool;

    // �ش� �����ʰ� ���� �� �ִ� �ִ� ���� ��� �ѵ�ġ�Դϴ�.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Pool")
    int32 MaxMonsterCount = 5;

    // �÷��̾ ���� ���� ���� ������ �� �Ҽ� ���͵��� ������ ����� �ݹ��Դϴ�.
    UFUNCTION()
    void OnSensorOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // �÷��̾ ���� ������ �־����� �� ���͵��� �޸�(��Ȱ��) ���·� ��ȯ�ϴ� �ݹ��Դϴ�.
    UFUNCTION()
    void OnSensorOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // ���� ��� �� ��ȣ�� ���޹޾� ��Ȱ(������) Ÿ�̸Ӹ� ������ŵ�ϴ�.
    UFUNCTION()
    void HandleMonsterDeath(AEnemy* DeadMonster);

    // ��Ȱ Ÿ�̸Ӱ� ����Ǿ��� �� �׾��� ���͸� ���ο� ��ġ�� �ٽ� ��ġ�ϰ� Ȱ��ȭ��ŵ�ϴ�.
    UFUNCTION()
    void RespawnMonster(AEnemy* MonsterToRespawn);

    // ���� �÷��̾ ������ ���� ���� ���� ���� �ִ����� ��Ÿ���� �ܺ� Ȯ�ο� �÷����Դϴ�.
    bool bIsPlayerInZone = false;

    // 몬스터별 개별 리스폰 타이머 핸들 맵
    // 단일 FTimerHandle을 쓰면 SetTimer 호출마다 이전 타이머가 덮어써져 마지막 1마리만 재스폰됩니다.
    // TMap으로 몬스터마다 독립된 핸들을 유지합니다.
    TMap<AEnemy*, FTimerHandle> RespawnTimerHandles;
};