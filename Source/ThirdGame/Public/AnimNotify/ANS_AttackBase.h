#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "ANS_AttackBase.generated.h"

// 히트 트레이스 기반 공격 AnimNotifyState의 베이스 클래스입니다.
// 몽타주 에디터에서 HitType 태그를 지정하면, 이 공격에 맞은 캐릭터가
// 해당 타입의 히트 리액션 애니메이션을 재생합니다.
UCLASS(Abstract)
class THIRDGAME_API UANS_AttackBase : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// 몽타주 에디터에서 설정하는 히트 리액션 타입 태그입니다. (예: HitType.Launch, HitType.Knockback)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|HitReaction")
	FGameplayTag HitType;
};
