#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "ShopSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UShopWidget;
class UItemTooltipWidget;

UCLASS()
class THIRDGAME_API UShopSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetItemData(FItemData InData);
	void SetEmpty();
	void Init(UShopWidget* InShopWidget);

protected:
	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidget))
	UImage* IconImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PriceText;

private:
	FItemData ItemData;

	UPROPERTY()
	UShopWidget* OwnerShopWidget;

	UPROPERTY()
	UItemTooltipWidget* CachedTooltip = nullptr;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UItemTooltipWidget> TooltipClass;
};
