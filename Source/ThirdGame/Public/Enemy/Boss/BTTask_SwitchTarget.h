#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SwitchTarget.generated.h"

// 공격 후 범위 내 다른 플레이어로 타겟을 전환하는 BT 태스크입니다.
// 즉시 Succeeded를 반환하므로 BT 옵저버 abort 문제가 발생하지 않습니다.
UCLASS()
class THIRDGAME_API UBTTask_SwitchTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SwitchTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	// 타겟 전환 탐색 반경 (이 범위 안의 플레이어 중 이전 타겟 외 랜덤으로 전환)
	UPROPERTY(EditAnywhere, Category = "Target")
	float TargetSwitchRange = 2000.f;

	// 블랙보드 TargetPlayer 키
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector PlayerKey;
};
