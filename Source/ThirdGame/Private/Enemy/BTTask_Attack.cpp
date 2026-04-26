// =========================================================================================
// BTTask_Attack.cpp
//
// [파일 역할]
// AI 비헤이비어 트리에서 적의 기본 공격(BaseAttack)을 실행하도록 지시하는 커스텀 BTTask입니다.
// ExecuteTask에서 공격을 시작하고, TickTask에서 bCanAttack이 복구될 때까지 대기한 뒤
// 태스크를 완료 처리합니다.
// =========================================================================================

#include "BTTask_Attack.h"
#include "AIController.h"
#include "Enemy.h"
#include "GameFramework/CharacterMovementComponent.h"

// 비헤이비어 트리 에디터에서 표시될 노드 이름을 설정하고 TickTask를 활성화합니다.
UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("BaseAttack");
	bNotifyTick = true;
}

// 적의 BaseAttack을 시작하고 공격 쿨다운이 끝날 때까지 InProgress 상태를 유지합니다.
EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	AEnemy* Enemy = Cast<AEnemy>(AIC->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	// 공격 몽타주 재생 및 쿨다운 타이머를 시작합니다.
	Enemy->BaseAttack();
	bNotifyTick = true;

	return EBTNodeResult::InProgress;
}

// 매 프레임 bCanAttack이 복구되면 태스크를 완료 처리합니다.
void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return;

	AEnemy* Enemy = Cast<AEnemy>(AIC->GetPawn());
	if (Enemy)
	{
		// 쿨다운이 끝나 bCanAttack이 복구되면 다음 노드로 진행합니다.
		if (Enemy->bCanAttack == true)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}
