#pragma once

#include "CoreMinimal.h"
#include "ItemEffectBase.h" 
#include "ItemEffect_StatPotion.generated.h"


UCLASS(Blueprintable, EditInlineNew)
class THIRDGAME_API UItemEffect_StatPotion : public UItemEffectBase
{
	GENERATED_BODY()

public:
	// ----------------------------------------------------------------
	// 데이터 테이블에서 기획자가 직접 입력할 수치들
	// ----------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion Effect")
	float HealAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion Effect")
	float ManaAmount = 0.0f;

	// 원석님이 추가하신 공격력 버프 수치!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion Effect")
	float AttackBoostAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion Effect")
	bool bIsPercentage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Effect")
	float BuffDuration = 10.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Effect")
	FName ItemRowName;

public:
	// 부모의 가상 함수(ExecuteItemEffect)를 C++에서 직접 덮어씁니다(Override).
	virtual void ExecuteItemEffect_Implementation(APawn* User) override;

	virtual bool CanUseItem_Implementation(APawn* User) override;
};