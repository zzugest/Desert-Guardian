#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_GroundSlamSocket.generated.h"

// AN_GroundSlam의 원본 로직 (소켓 XY 기준 이펙트 + 데미지 트레이스)
// 보스 점프 공격 외 다른 공격에서 소켓 기반으로 동작해야 할 때 이 클래스를 사용합니다.
UCLASS()
class THIRDGAME_API UAN_GroundSlamSocket : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 소켓이 있는 컴포넌트에서 읽어올 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slam Settings")
	FName SocketName = TEXT("WeaponSocket");

	// 스폰할 나이아가라 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slam Settings")
	class UNiagaraSystem* SlamEffect;

	// 데미지 구체 트레이스 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slam Settings")
	float DamageRadius = 150.0f;

	// 적용할 데미지 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slam Settings")
	float SlamDamage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slam")
	FVector EffectScale = FVector(1.0f, 1.0f, 1.0f);

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
