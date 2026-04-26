// =========================================================================================
// BTTask_FindPatrolLocation.cpp
//
// [파일 역할]
// AI 비헤이비어 트리에서 순찰 경로를 동적으로 생성하는 커스텀 BTTask입니다.
// 적의 스폰 위치(HomeLocation)와 PatrolRadius 범위 내에서 내비게이션 메시가
// 도달 가능한 임의의 위치를 찾아 블랙보드에 저장합니다.
// =========================================================================================

#include "BTTask_FindPatrolLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Enemy.h"

// 비헤이비어 트리 에디터에서 표시될 노드 이름을 설정합니다.
UBTTask_FindPatrolLocation::UBTTask_FindPatrolLocation()
{
    NodeName = TEXT("Find Patrol Location");
}

// 스폰 위치 기준 PatrolRadius 범위 내 도달 가능한 임의 좌표를 찾아 블랙보드에 저장합니다.
EBTNodeResult::Type UBTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    // 스폰 위치(HomeLocation)와 순찰 반경을 읽기 위해 Enemy 레퍼런스가 필요합니다.
    AEnemy* Monster = Cast<AEnemy>(AIController->GetPawn());
    if (!Monster) return EBTNodeResult::Failed;

    // 내비게이션 메시(NavMesh) 위에서만 도달 가능한 좌표를 탐색합니다.
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSystem) return EBTNodeResult::Failed;

    FNavLocation RandomLocation;
    bool bFound = NavSystem->GetRandomReachablePointInRadius(Monster->HomeLocation, Monster->PatrolRadius, RandomLocation);

    if (!bFound) return EBTNodeResult::Failed;

    // MoveTo 태스크가 이 좌표를 읽어 이동할 수 있도록 블랙보드에 기록합니다.
    OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), RandomLocation.Location);

    return EBTNodeResult::Succeeded;
}
