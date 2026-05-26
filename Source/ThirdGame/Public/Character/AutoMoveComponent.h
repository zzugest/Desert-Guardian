#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoMoveComponent.generated.h"

class UDecalComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class THIRDGAME_API UAutoMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAutoMoveComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 지정한 위치를 향해 NavMesh 경로를 계산하고 자동이동을 시작합니다.
	UFUNCTION(BlueprintCallable, Category = "AutoMove")
	void StartAutoMove(FVector TargetLocation);

	// 자동이동을 중단하고 경로 데칼을 모두 제거합니다.
	UFUNCTION(BlueprintCallable, Category = "AutoMove")
	void StopAutoMove();

	// 현재 자동이동 중인지 여부입니다.
	UPROPERTY(BlueprintReadOnly, Category = "AutoMove")
	bool bIsAutoMoving = false;

	// 경로 시각화에 사용할 데칼 머티리얼입니다. 에디터에서 할당합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMove|Visual")
	UMaterialInterface* PathDecalMaterial = nullptr;

	// 데칼 사이 간격(유닛)입니다. 이 간격마다 데칼을 하나씩 스폰합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMove|Visual")
	float DecalSpacing = 150.0f;

	// 데칼 반지름(유닛)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMove|Visual")
	float DecalRadius = 40.0f;

	// 웨이포인트 하나를 통과했다고 판정하는 거리(유닛)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMove|Movement")
	float WaypointAcceptanceRadius = 80.0f;

	// 이 시간(초) 동안 제자리이면 자동이동을 중단합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMove|Movement")
	float StuckMaxTime = 4.0f;

	// 제자리 판정 기준 거리(유닛). 1초 동안 이 거리 미만으로 이동하면 제자리로 봅니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMove|Movement")
	float StuckDistanceThreshold = 30.0f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// 최종 목적지입니다.
	FVector Destination = FVector::ZeroVector;

	// NavMesh에서 계산된 경로 웨이포인트 목록입니다.
	TArray<FVector> PathPoints;

	// 현재 향하고 있는 웨이포인트 인덱스입니다.
	int32 CurrentPathIndex = 0;

	// 스폰된 경로 데칼 목록입니다.
	UPROPERTY()
	TArray<UDecalComponent*> PathDecals;

	// 제자리 감지용 이전 위치 스냅샷입니다. (1초마다 갱신)
	FVector LastStuckCheckPos = FVector::ZeroVector;

	// 제자리 누적 시간입니다.
	float StuckTimer = 0.0f;

	// 1초 스냅샷 간격 누산기입니다.
	float StuckCheckAccumulator = 0.0f;

	// 1초 주기 경로 재계산 타이머 핸들입니다.
	FTimerHandle RecalcTimerHandle;

	// NavMesh로 경로를 재계산하고 경로 데칼을 갱신합니다.
	void RecalcPath();

	// 경로 웨이포인트를 따라 데칼을 스폰합니다.
	void SpawnPathDecals();

	// 기존 경로 데칼을 모두 제거합니다.
	void ClearPathDecals();
};
