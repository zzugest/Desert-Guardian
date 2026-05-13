// =========================================================================================
// TargetingComponent.h
//
// [파일 역할]
// 카메라 전방으로 구체를 스윕하여 화면 중앙에 가장 가까운 적을 자동으로 탐색하고
// 타겟 마커를 표시하는 자동 타겟팅 컴포넌트입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THIRDGAME_API UTargetingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTargetingComponent();

    // 0.1초 간격으로 타겟 스캔을 반복 수행합니다.
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 현재 시스템에 의해 선택된 최우선 타겟 액터입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
    AActor* CurrentTarget;

    // 카메라 전방으로 구체를 스윕할 최대 거리입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    float TraceDistance = 3000.0f;

    // 전방 스윕 구체의 반경입니다. 클수록 옆에 있는 적도 감지합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    float SweepRadius = 500.0f;

    // true로 설정하면 PIE에서 타겟팅 범위를 시각적으로 확인할 수 있습니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Debug")
    bool bShowDebug = false;

private:
    // 카메라 전방(구체 스윕)과 시야 차단(벽 트레이스)을 검사하여 화면 중앙에 가장 가까운 적을 찾아 설정합니다.
    void FindTarget();
};