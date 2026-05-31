
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestLogWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UButton;
class UQuestComponent;

/**
 * 
 */
UCLASS()
class THIRDGAME_API UQuestLogWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION()
    void UpdateQuestLog();

public:
    
    UPROPERTY(meta = (BindWidget))
    class URichTextBlock* QuestListText;

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* QuestBackground;

    UPROPERTY(meta = (BindWidget))
    UButton* AutoMoveButton;

    UFUNCTION()
    void OnAutoMoveClicked();

private:
    UPROPERTY()
    UQuestComponent* CachedQuestComp;
};
