#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_FireProjectile.generated.h"

UCLASS()
class THIRDGAME_API UAnimNotify_FireProjectile : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 에디터에서 기획자가 입력할 스킬 ID (예: "IceShoot")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Data")
	FName SkillID;

	// 발사될 소켓 이름 (기본값을 Hand_Left로 두면 편합니다)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Data")
	FName SocketName = FName("Hand_Left");

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};