// =========================================================================================
// QuestDialogueWidget.cpp
//
// [파일 역할]
// NPC에서 퀘스트를 수락하거나 완료할 때 표시되는 대화창 UI 클래스입니다.
// 세 가지 모드를 지원합니다:
//   - 일반 대화(StartDialogue): 퀘스트 소개 → 수락/거절 선택 → 수락/거절 대사
//   - 완료 대화(StartCompletionDialogue): 완료 대사 → 마지막 장에서 보상 UI 표시 → 퀘스트 완료 처리
//   - 안내 대화(StartNoticeDialogue): 진행 중 NPC가 짧은 메시지만 전달
// 대화창이 열릴 때 PlayerHUD를 숨기고, 닫힐 때 복구합니다.
// =========================================================================================

#include "NPC/Quest/QuestDialogueWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "NPC/Quest/QuestComponent.h"
#include "TimerManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/HorizontalBox.h"
#include "NPC/Quest/QuestRewardSlotWidget.h"
#include "Item/ItemData.h"
#include "MyGameInstance.h"
#include "GlobalUI/GlobalUIData.h"
#include "Character/HUDRootWidget.h"
#include "Character/PlayerHUDWidget.h"

// HUDRoot에서 PlayerHUD를 찾아 Visibility를 설정하는 내부 헬퍼입니다.
static void SetPlayerHUDVisibility_Quest(UObject* WorldContext, ESlateVisibility InVisibility)
{
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(WorldContext, FoundWidgets, UHUDRootWidget::StaticClass(), true);

    if (FoundWidgets.Num() > 0)
    {
        UHUDRootWidget* HUDRoot = Cast<UHUDRootWidget>(FoundWidgets[0]);
        if (HUDRoot && HUDRoot->PlayerHUD)
        {
            HUDRoot->PlayerHUD->SetVisibility(InVisibility);
        }
    }
}

// 버튼 이벤트를 바인딩하고 대화창이 열릴 때 PlayerHUD를 숨깁니다.
void UQuestDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (AcceptButton) AcceptButton->OnClicked.AddDynamic(this, &UQuestDialogueWidget::OnAcceptClicked);
    if (DeclineButton) DeclineButton->OnClicked.AddDynamic(this, &UQuestDialogueWidget::OnDeclineClicked);

    // 대화창이 열릴 때 PlayerHUD만 숨깁니다. MinimapWidget은 그대로 유지됩니다.
    SetPlayerHUDVisibility_Quest(this, ESlateVisibility::Hidden);
}

// 대화창이 닫힐 때 PlayerHUD를 복구합니다.
void UQuestDialogueWidget::NativeDestruct()
{
    SetPlayerHUDVisibility_Quest(this, ESlateVisibility::Visible);
    Super::NativeDestruct();
}

// 퀘스트 소개 대화를 시작합니다. 대사를 모두 읽으면 수락/거절 버튼이 나타납니다.
void UQuestDialogueWidget::StartDialogue(FName InQuestID, const FQuestData& QuestData, UQuestComponent* InQuestComp)
{
    bIsChoiceMade    = false;
    bIsNoticeMode    = false;
    bIsCompletionMode = false;

    CurrentQuestID  = InQuestID;
    CachedQuestComp = InQuestComp;
    CachedQuestData = QuestData;

    CurrentDialogues = QuestData.StartDialogues;
    AcceptDialogues  = QuestData.AcceptDialogues;
    DeclineDialogues = QuestData.DeclineDialogues;
    CurrentPageIndex = 0;

    // 대사를 다 읽기 전까지는 수락/거절 버튼을 숨깁니다.
    if (AcceptButton)    AcceptButton->SetVisibility(ESlateVisibility::Hidden);
    if (DeclineButton)   DeclineButton->SetVisibility(ESlateVisibility::Hidden);

    if (RewardContainer)
    {
        RewardContainer->SetVisibility(ESlateVisibility::Hidden);
        RewardContainer->ClearChildren();
    }

    NextDialoguePage();
}

// 클릭 시 다음 대사 페이지를 표시하거나, 모든 대사가 끝나면 다음 단계로 넘어갑니다.
void UQuestDialogueWidget::NextDialoguePage()
{
    // 수락/거절 버튼이 표시된 상태에서는 클릭을 무시합니다.
    if (AcceptButton && AcceptButton->IsVisible())
    {
        return;
    }

    // 아직 표시할 대사가 남아 있으면 텍스트를 갱신합니다.
    if (CurrentPageIndex < CurrentDialogues.Num())
    {
        if (DialogueText)
        {
            FString RawString = CurrentDialogues[CurrentPageIndex].ToString();
            FString FormattedString = RawString.Replace(TEXT("\\n"), TEXT("\n"));
            DialogueText->SetText(FText::FromString(FormattedString));
        }

        // 완료 모드의 마지막 페이지에서 보상 UI를 미리 표시합니다.
        if (CurrentPageIndex == CurrentDialogues.Num() - 1 && bIsCompletionMode)
        {
            if (RewardContainer && RewardSlotClass && CachedQuestComp)
            {
                RewardContainer->SetVisibility(ESlateVisibility::Visible);
                RewardContainer->ClearChildren();
                GenerateRewardUI();
            }
        }
        CurrentPageIndex++;
        return;
    }

    // ─── 모든 대사가 끝난 경우 ───

    // 안내 모드는 대사 종료 즉시 창을 닫습니다.
    if (bIsNoticeMode)
    {
        CloseDialogue();
        return;
    }

    // 완료 모드는 퀘스트를 최종 완료 처리하고 창을 닫습니다.
    if (bIsCompletionMode)
    {
        if (CachedQuestComp)
        {
            CachedQuestComp->CompleteQuest(CurrentQuestID);
        }
        CloseDialogue();
        return;
    }

    // 수락/거절 선택 후 대사(AcceptDialogues/DeclineDialogues)가 끝나면 창을 닫습니다.
    if (bIsChoiceMade)
    {
        CloseDialogue();
        return;
    }

    // 모든 소개 대사를 읽었으므로 수락/거절 버튼과 보상 UI를 표시합니다.
    if (AcceptButton)  AcceptButton->SetVisibility(ESlateVisibility::Visible);
    if (DeclineButton) DeclineButton->SetVisibility(ESlateVisibility::Visible);

    if (RewardContainer && RewardSlotClass && CachedQuestComp)
    {
        RewardContainer->SetVisibility(ESlateVisibility::Visible);
        RewardContainer->ClearChildren();
        GenerateRewardUI();
    }
}

// 수락 버튼 클릭: 퀘스트를 등록하고 수락 대사로 전환합니다.
void UQuestDialogueWidget::OnAcceptClicked()
{
    bIsChoiceMade = true;

    if (AcceptButton)    AcceptButton->SetVisibility(ESlateVisibility::Hidden);
    if (DeclineButton)   DeclineButton->SetVisibility(ESlateVisibility::Hidden);
    if (RewardContainer) RewardContainer->SetVisibility(ESlateVisibility::Hidden);

    if (CachedQuestComp)
    {
        CachedQuestComp->AcceptQuest(CurrentQuestID);
    }

    // 수락 대사를 처음부터 읽기 시작합니다.
    CurrentDialogues = AcceptDialogues;
    CurrentPageIndex = 0;
    NextDialoguePage();
}

// 거절 버튼 클릭: 거절 대사로 전환합니다.
void UQuestDialogueWidget::OnDeclineClicked()
{
    bIsChoiceMade = true;

    if (AcceptButton)    AcceptButton->SetVisibility(ESlateVisibility::Hidden);
    if (DeclineButton)   DeclineButton->SetVisibility(ESlateVisibility::Hidden);
    if (RewardContainer) RewardContainer->SetVisibility(ESlateVisibility::Hidden);

    // 거절 대사를 처음부터 읽기 시작합니다.
    CurrentDialogues = DeclineDialogues;
    CurrentPageIndex = 0;
    NextDialoguePage();
}

// 대화창을 제거하고 게임 전용 입력 모드와 이동 입력을 복구합니다.
void UQuestDialogueWidget::CloseDialogue()
{
    RemoveFromParent();

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    PC->SetInputMode(FInputModeGameOnly());
    PC->SetShowMouseCursor(false);
    PC->SetIgnoreMoveInput(false);
}

// 퀘스트 진행 중 NPC가 짧은 메시지만 전달하는 안내 모드를 시작합니다.
void UQuestDialogueWidget::StartNoticeDialogue(FText NoticeText)
{
    bIsChoiceMade = false;
    bIsNoticeMode = true;

    CurrentDialogues.Empty();
    CurrentDialogues.Add(NoticeText);
    CurrentPageIndex = 0;

    if (AcceptButton)  AcceptButton->SetVisibility(ESlateVisibility::Hidden);
    if (DeclineButton) DeclineButton->SetVisibility(ESlateVisibility::Hidden);

    NextDialoguePage();
}

// 보상 UI를 생성합니다. 골드 → 아이템 순서로 HorizontalBox에 슬롯을 추가합니다.
void UQuestDialogueWidget::GenerateRewardUI()
{
    if (!RewardContainer || !CachedQuestComp->ItemDataTable) return;

    // GlobalUIData에서 골드 아이콘 텍스처를 가져옵니다.
    UTexture2D* GoldImage = nullptr;
    UMyGameInstance* GameInst = Cast<UMyGameInstance>(GetGameInstance());
    if (GameInst && GameInst->GlobalUIData)
    {
        GoldImage = GameInst->GlobalUIData->GoldIcon;
    }

    // 골드 보상 슬롯을 먼저 추가합니다.
    if (CachedQuestData.RewardGold > 0)
    {
        UQuestRewardSlotWidget* GoldSlot = CreateWidget<UQuestRewardSlotWidget>(this, RewardSlotClass);
        if (GoldSlot)
        {
            GoldSlot->SetupSlot(GoldImage, TEXT("Gold"), CachedQuestData.RewardGold);
            RewardContainer->AddChildToHorizontalBox(GoldSlot);
        }
    }

    // 아이템 보상 슬롯을 순서대로 추가합니다.
    for (const FQuestItemReward& RewardNode : CachedQuestData.RewardItems)
    {
        FItemData* FoundItem = CachedQuestComp->ItemDataTable->FindRow<FItemData>(
            RewardNode.ItemRowName, TEXT("RewardUI"));
        if (FoundItem)
        {
            UQuestRewardSlotWidget* ItemSlot = CreateWidget<UQuestRewardSlotWidget>(this, RewardSlotClass);
            if (ItemSlot)
            {
                ItemSlot->SetupSlot(FoundItem->ItemIcon, FoundItem->ItemName, RewardNode.Quantity);
                RewardContainer->AddChildToHorizontalBox(ItemSlot);
            }
        }
    }
}

// 완료 대화 모드를 시작합니다. CompletedDialogues를 읽은 후 퀘스트를 최종 완료 처리합니다.
void UQuestDialogueWidget::StartCompletionDialogue(FName InQuestID, const FQuestData& QuestData, UQuestComponent* InQuestComp)
{
    bIsChoiceMade    = false;
    bIsNoticeMode    = false;
    bIsCompletionMode = true;

    CurrentQuestID  = InQuestID;
    CachedQuestComp = InQuestComp;
    CachedQuestData = QuestData;

    CurrentDialogues = QuestData.CompletedDialogues;
    CurrentPageIndex = 0;

    // 완료 모드에서는 수락/거절 버튼이 필요 없습니다.
    if (AcceptButton)    AcceptButton->SetVisibility(ESlateVisibility::Hidden);
    if (DeclineButton)   DeclineButton->SetVisibility(ESlateVisibility::Hidden);
    if (RewardContainer) RewardContainer->SetVisibility(ESlateVisibility::Hidden);

    NextDialoguePage();
}
