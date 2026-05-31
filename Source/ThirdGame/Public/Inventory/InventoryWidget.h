
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "GameWidgetBase.h"
#include "Components/Button.h"
#include "InventoryWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryClosed);

class UTileView;

UCLASS()
class THIRDGAME_API UInventoryWidget : public UGameWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void UpdateUI(const TArray<FItemData>& Content, int32 Capacity);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInventoryClosed OnWindowClosed;

protected:

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UTileView* InventoryTileView;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Close;

	UFUNCTION()
	void OnCloseButtonClicked();

	UPROPERTY()
	TArray<class UInventoryItemObject*> CachedItemObjects;

};
