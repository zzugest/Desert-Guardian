#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

UCLASS()
class THIRDGAME_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	//  게임 시작 시 한 번 실행됨 
	virtual void NativeInitializeAnimation() override;

	//  매 프레임 실행됨 (Tick과 동일, 변수 업데이트용)
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 이 애니메이션을 소유하고 있는 몬스터 본체
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	class AEnemy* OwnerEnemy;

	//  이동 속도 (Idle/Run 전환용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float Speed;

	// 사망 여부 (죽음 애니메이션 전환용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsDead;
};