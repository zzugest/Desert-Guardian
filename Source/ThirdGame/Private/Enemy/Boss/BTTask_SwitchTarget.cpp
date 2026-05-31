#include "Enemy/Boss/BTTask_SwitchTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/MyCharacter.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UBTTask_SwitchTarget::UBTTask_SwitchTarget()
{
	NodeName = TEXT("Switch Target");
	PlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SwitchTarget, PlayerKey), AActor::StaticClass());
}

// 범위 내 이전 타겟 외 다른 플레이어로 랜덤 전환합니다. 즉시 Succeeded를 반환합니다.
EBTNodeResult::Type UBTTask_SwitchTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BBComp) return EBTNodeResult::Failed;

	ACharacter* BossChar = Cast<ACharacter>(AICon->GetPawn());
	if (!BossChar) return EBTNodeResult::Failed;

	// PlayerKey.SelectedKeyName 대신 하드코딩된 키를 사용합니다.
	// OnTargetDetected(Enemy.cpp)와 LaunchToAir(BossMonster.cpp)도 동일한 키를 씁니다.
	static const FName TargetKey = TEXT("TargetPlayer");
	AMyCharacter* PrevTarget = Cast<AMyCharacter>(BBComp->GetValueAsObject(TargetKey));

	// 범위 내 이전 타겟 외 생존한 플레이어를 수집합니다.
	TArray<AActor*> AllPlayers;
	UGameplayStatics::GetAllActorsOfClass(BossChar->GetWorld(), AMyCharacter::StaticClass(), AllPlayers);

	TArray<AMyCharacter*> Candidates;
	for (AActor* Actor : AllPlayers)
	{
		AMyCharacter* PC = Cast<AMyCharacter>(Actor);
		if (!PC) continue;
		if (PC == PrevTarget) continue;
		if (PC->HasStateTag("State.Dead")) continue;
		if (FVector::Dist(BossChar->GetActorLocation(), PC->GetActorLocation()) > TargetSwitchRange) continue;
		Candidates.Add(PC);
	}

	if (Candidates.Num() > 0)
	{
		AMyCharacter* NewTarget = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
		BBComp->SetValueAsObject(TargetKey, NewTarget);
		UE_LOG(LogTemp, Warning, TEXT("[Boss] 타겟 전환: [%s] → [%s]"),
			PrevTarget ? *PrevTarget->GetName() : TEXT("없음"),
			*NewTarget->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Boss] 타겟 전환 후보 없음 → 현재 타겟 유지: [%s]"),
			PrevTarget ? *PrevTarget->GetName() : TEXT("없음"));
	}

	return EBTNodeResult::Succeeded;
}
