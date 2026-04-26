#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_ChangePlayRate.generated.h"

UCLASS()
class THIRDGAME_API UANS_ChangePlayRate : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// 에디터에서 우리가 마음대로 속도를 입력할 수 있게 열어둡니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayRate")
	float TargetPlayRate = 0.5f;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};