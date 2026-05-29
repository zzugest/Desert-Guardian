#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TurnToPlayer.generated.h"

// 90도, 180도 회전 애니메이션들을 담을 구조체 (USTRUCT로 만들어 블루프린트에서 편집하기 쉽게 만듭니다.)
USTRUCT(BlueprintType)
struct FRotateMontages
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* TurnL90;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* TurnR90;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* TurnL180;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* TurnR180;
};

/**
 * 플레이어를 향해 제자리에서 회전하는 비헤이비어 트리 태스크 (루트 모션 기반)
 */
UCLASS()
class THIRDGAME_API UBTTask_TurnToPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_TurnToPlayer();

protected:
	// 태스크가 시작될 때 실행되는 함수
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 태스크가 진행 중일 때 프레임마다 실행되는 함수 (애니메이션이 끝났는지 확인용)
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// 블랙보드에서 플레이어 액터를 가져오기 위한 키
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector PlayerKey;

	// 원석님이 가지고 계신 4개의 회전 몽타주를 등록할 변수
	UPROPERTY(EditAnywhere, Category = "Animation")
	FRotateMontages RotateMontages;


	// 몽타주가 끝났을 때 자물쇠를 풀어줄 콜백 함수
	UFUNCTION()
	void OnTurnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 현재 회전 중인지 확인하는 자물쇠
	bool bIsTurning;

	// 현재 태스크를 소유한 Behavior Tree 컴포넌트를 기억 (FinishLatentTask 호출용)
	UBehaviorTreeComponent* CachedOwnerComp;
};