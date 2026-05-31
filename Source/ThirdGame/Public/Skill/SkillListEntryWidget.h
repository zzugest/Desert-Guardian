#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h" 
#include "SkillListEntryWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
class USkillDragVisual;
class UCombatComponent;

UCLASS()
class THIRDGAME_API USkillListEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void UpdateUI(FName SkillID);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UImage* SkillIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SkillName;

	UPROPERTY(BlueprintReadOnly)
	FName CurrentSkillID;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drag Drop")
	TSubclassOf<USkillDragVisual> DragVisualClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
	TSubclassOf<class USkillTooltipWidget> SkillTooltipClass;

private:
	UPROPERTY()
	UCombatComponent* CachedCombatComp = nullptr;
};
