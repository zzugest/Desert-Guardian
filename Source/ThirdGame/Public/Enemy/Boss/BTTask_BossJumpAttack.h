// BTTask_BossJumpAttack.h

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossJumpAttack.generated.h"

UCLASS()
class THIRDGAME_API UBTTask_BossJumpAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossJumpAttack();

protected:
	// 비헤이비어 트리가 이 노드를 실행할 때 부르는 핵심 함수입니다.
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};