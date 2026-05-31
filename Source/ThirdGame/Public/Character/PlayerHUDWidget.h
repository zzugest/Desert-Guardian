
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.h"
#include "PlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class THIRDGAME_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void UpdateState(float CurrentHP, float MaxHP, float CurrentMP, float MaxMP, float CurrentSP, float MaxSP);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ManaBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* SPBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GoldText;

	UFUNCTION()
	void OnPlayerMoneyChanged(int32 NewGold);
};
