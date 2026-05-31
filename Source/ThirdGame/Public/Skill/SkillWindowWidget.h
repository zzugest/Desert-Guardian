#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "SkillWindowWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillWindowClosed);

class UUniformGridPanel;

UCLASS()
class THIRDGAME_API USkillWindowWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* SkillContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> SkillEntryClass;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Close;

	UFUNCTION()
	void OnCloseButtonClicked();

public:
	void RefreshUI();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSkillWindowClosed OnWindowClosed;

};
