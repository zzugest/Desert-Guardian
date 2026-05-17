#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_WeaponTrace.generated.h"

UCLASS()
class THIRDGAME_API UAnimNotifyState_WeaponTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// 기획자가 입력할 공격 ID (예: Jump_Loop)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Data")
	FName AttackID;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};