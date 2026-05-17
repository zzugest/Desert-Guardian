#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindPatrolLocation.generated.h"

UCLASS()
class THIRDGAME_API UBTTask_FindPatrolLocation : public UBTTask_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTTask_FindPatrolLocation();

protected:
    // 태스크가 실행될 때 작동하는 메인 함수
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};