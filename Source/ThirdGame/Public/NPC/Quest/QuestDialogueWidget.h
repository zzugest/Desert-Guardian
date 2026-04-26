// =========================================================================================
// QuestDialogueWidget.h
//
// [역할 설명]
// NPC 상호작용 시 화면에 퀘스트 수락 대화, 수락/거절 선택창, 일반적인 알림 메시지 등을 처리하는 대화 UI 위젯 클래스입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TimerHandle.h"
#include "QuestData.h"
#include "QuestDialogueWidget.generated.h"

class UTextBlock;
class UButton;
class UQuestComponent;
class UHorizontalBox;
class UQuestRewardSlotWidget;

UCLASS()
class THIRDGAME_API UQuestDialogueWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    // UI 생성 시 버튼 이벤트를 바인딩하고 PlayerHUD를 숨깁니다.
    virtual void NativeConstruct() override;

    // 대화창이 닫힐 때 PlayerHUD를 복구합니다.
    virtual void NativeDestruct() override;

    bool bIsCompletionMode = false;

public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DialogueText;

    UPROPERTY(meta = (BindWidget))
    UButton* AcceptButton;

    UPROPERTY(meta = (BindWidget))
    UButton* DeclineButton;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(FName InQuestID, const FQuestData& QuestData, UQuestComponent* InQuestComp);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NextDialoguePage();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartNoticeDialogue(FText NoticeText);

    UFUNCTION(BlueprintCallable)
    void StartCompletionDialogue(FName InQuestID, const FQuestData& QuestData, class UQuestComponent* InQuestComp);

    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* RewardContainer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UQuestRewardSlotWidget> RewardSlotClass;

private:
    UFUNCTION()
    void OnAcceptClicked();

    UFUNCTION()
    void OnDeclineClicked();

    void CloseDialogue();

    TArray<FText> CurrentDialogues;
    int32 CurrentPageIndex = 0;

    FName CurrentQuestID;

    UPROPERTY()
    UQuestComponent* CachedQuestComp;

    TArray<FText> AcceptDialogues;
    TArray<FText> DeclineDialogues;

    bool bIsChoiceMade = false;
    bool bIsNoticeMode = false;

    FQuestData CachedQuestData;

    void GenerateRewardUI();
};
