#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuffListWidget.generated.h"

class UHorizontalBox;
class UBuffIconWidget;
class USkillComponent;
class UCombatComponent;

UCLASS()
class THIRDGAME_API UBuffListWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* BuffBox;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UBuffIconWidget> BuffIconClass;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	class UDataTable* SkillDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	class UDataTable* ItemDataTable;

public:
	UFUNCTION()
	void UpdateBuffList();

	UFUNCTION()
	void LogBuffUIStatus();

private:
	UPROPERTY()
	USkillComponent* CachedSkillComp;

	UPROPERTY()
	UCombatComponent* CachedCombatComp;

	FTimerHandle BuffUILogTimerHandle;
};
