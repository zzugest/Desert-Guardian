// =========================================================================================
// BTTask_BossJumpAttack.cpp
//
// [파일 역할]
// AI 비헤이비어 트리에서 보스의 점프 공격을 실행하도록 지시하는 커스텀 BTTask입니다.
// 비헤이비어 트리가 이 태스크를 활성화하면 BossMonster::ExecuteJumpAttack()을 호출합니다.
// =========================================================================================

#include "Enemy/Boss/BTTask_BossJumpAttack.h"
#include "AIController.h"
#include "Enemy/Boss/BossMonster.h"

// 비헤이비어 트리 에디터에서 표시될 노드 이름을 설정합니다.
UBTTask_BossJumpAttack::UBTTask_BossJumpAttack()
{
	NodeName = TEXT("Boss Jump Attack");
}

// 보스 액터를 가져와 점프 공격 함수를 호출하고 태스크 성공을 반환합니다.
EBTNodeResult::Type UBTTask_BossJumpAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	ABossMonster* Boss = Cast<ABossMonster>(AICon->GetPawn());
	if (!Boss) return EBTNodeResult::Failed;

	Boss->ExecuteJumpAttack();

	return EBTNodeResult::Succeeded;
}
