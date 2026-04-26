#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_TrackTarget.generated.h"

UCLASS()
class THIRDGAME_API UANS_TrackTarget : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// 에디터에서 보스의 회전 속도를 마음대로 조절할 수 있습니다. (기본값 800은 꽤 빠릅니다!)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking")
	float TrackingSpeed = 800.0f;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};