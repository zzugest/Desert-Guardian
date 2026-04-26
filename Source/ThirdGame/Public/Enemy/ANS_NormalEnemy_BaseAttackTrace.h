// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_NormalEnemy_BaseAttackTrace.generated.h"

/**
 * 
 */
UCLASS()
class THIRDGAME_API UANS_NormalEnemy_BaseAttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//  애니메이션 창에서 기획자가 직접 입력할 소켓 이름들!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FName StartSocketName = TEXT("MeleeStart");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FName EndSocketName = TEXT("MeleeEnd");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float StateDamage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceRadius = 20.0f; // 트레이스 구체의 두께

	//  스테이트의 3대장 함수 오버라이드
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};