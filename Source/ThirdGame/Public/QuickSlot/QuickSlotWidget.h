
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h" 
#include "InventoryDragVisual.h"
#include "QuickSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UItemTooltipWidget;

UCLASS()
class UQuickSlotDragPayload : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	int32 SourceSlotIndex = -1;
};

UCLASS()
class THIRDGAME_API UQuickSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickSlot")
	int32 QuickSlotIndex = 0;

	UFUNCTION(BlueprintCallable)
	void SetItem(const FItemData& NewItem);

	UFUNCTION(BlueprintCallable)
	void SetKeyNumber(int32 Number);

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CountText;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* IconImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KeyText;

	UPROPERTY(EditDefaultsOnly, Category = "QuickSlot")
	TSubclassOf<UInventoryDragVisual> DragVisualClass;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	void UpdateSlotDisplay();

	UFUNCTION()
	void OnQuickSlotSystemUpdated();

	UPROPERTY(meta = (BindWidget))
	class UImage* CooldownOverlay;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownText;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
	TSubclassOf<UItemTooltipWidget> TooltipClass;

	UPROPERTY()
	UItemTooltipWidget* CachedTooltip;
};
