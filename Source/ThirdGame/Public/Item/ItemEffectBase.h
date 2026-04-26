#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemEffectBase.generated.h"

UCLASS(Blueprintable, Abstract, EditInlineNew) 
class THIRDGAME_API UItemEffectBase : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Effect")
	float CooldownTime = 0.0f;
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Effect")
	bool CanUseItem(APawn* User);
	virtual bool CanUseItem_Implementation(APawn* User) { return true; }

	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Effect")
	void ExecuteItemEffect(APawn* User);
	virtual void ExecuteItemEffect_Implementation(APawn* User) {  }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item Effect")
	FText UsageFailMessage;
};