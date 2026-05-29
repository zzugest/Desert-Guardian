#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "ANS_SetHitType.generated.h"

// 적의 공격 애니메이션 구간에 HitType 태그를 설정/해제하는 AnimNotifyState입니다.
// 이 구간에 맞은 플레이어는 태그에 맞는 히트 리액션 애니메이션을 재생합니다.
UCLASS()
class THIRDGAME_API UANS_SetHitType : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// 에디터에서 설정할 HitType 태그 (예: HitType.Knockback)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	FGameplayTag HitTag;

	// 공격 구간 시작 시 적에게 HitType 태그를 설정합니다.
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	// 공격 구간 종료 시 적의 HitType 태그를 초기화합니다.
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
