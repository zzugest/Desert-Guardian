#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "ItemTooltipWidget.generated.h"

class UTextBlock;

UCLASS()
class THIRDGAME_API UItemTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void InitTooltip(const FItemData& ItemInfo);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemDescText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemEffectText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ItemCooldownText = nullptr;
};
