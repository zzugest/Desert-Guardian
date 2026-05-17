#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_AttackHit.generated.h"

UCLASS()
class THIRDGAME_API UAnimNotify_AttackHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 에디터에서 기획자가 직접 입력할 수 있는 '데이터 테이블 행 이름(ID)'
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Data")
	FName AttackID;

	// 노티파이가 실행될 때 자동으로 불리는 함수
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};