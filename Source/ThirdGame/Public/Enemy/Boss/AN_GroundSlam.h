#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_GroundSlam.generated.h"

UCLASS()
class THIRDGAME_API UAN_GroundSlam : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 검이나 손에 뚫어둔 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slam Settings")
	FName SocketName = TEXT("WeaponSocket");

	// 소환할 나이아가라 이펙트 (바위 솟아남)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slam Settings")
	class UNiagaraSystem* SlamEffect;

	// 데미지 판정 구체의 반지름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slam Settings")
	float DamageRadius = 150.0f;

	// 입힐 데미지 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slam Settings")
	float SlamDamage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slam")
	FVector EffectScale = FVector(1.0f, 1.0f, 1.0f);

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};