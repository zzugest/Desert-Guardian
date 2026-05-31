#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillSlotWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class USkillComponent;
class UCombatComponent;
class USkillTooltipWidget;

UCLASS()
class THIRDGAME_API USkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Slot")
	int32 SlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Slot")
	FText KeyName;

	UFUNCTION(BlueprintCallable)
	void UpdateSlotInfo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop")
	TSubclassOf<class UUserWidget> DragVisualClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
	TSubclassOf<USkillTooltipWidget> SkillTooltipClass;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativePreConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidget))
	UImage* SkillIcon;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* CooldownBar;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* CooldownOverlay;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* CooldownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* KeyText;

private:
	UPROPERTY()
	USkillComponent* OwnerSkillComp;

	UPROPERTY()
	UCombatComponent* OwnerCombatComp;

	FName CurrentSkillID;

	void UpdateTooltip();
};
