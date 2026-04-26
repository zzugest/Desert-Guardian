// =========================================================================================
// TargetingComponent.h
//
// [역할 요약]
// 카메라 시점을 기준으로 화면 중앙에 가장 가깝고 시야가 확보된 적을 자동으로 식별하여 표적으로 설정하는 자동 타겟팅 컴포넌트 헤더입니다.
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

    // 매 프레임 타겟 스캔 로직을 반복 수행합니다.
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 현재 시스템에 의해 최우선 공격 추적 대상으로 포착된 액터 객체입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
    AActor* CurrentTarget;

    // 카메라 기준 타겟팅을 허용할 최대 탐지(락온) 거리입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    float TraceDistance = 3000.0f;

private:
    // 카메라 전방각(내적)과 가시성(레이 트레이스)을 검사하여 화면 중앙에 가장 근접한 적을 선별해냅니다.
    void FindTarget();
};