// =========================================================================================
// TalkDialogueWidget.cpp
//
// [파일 역할]
// 일반 NPC와 대화 시 대사를 한 줄씩 표시하는 대화창 위젯입니다.
// 창이 열릴 때 PlayerHUD를 숨기고, 닫힐 때 복구합니다(MinimapWidget은 유지).
// 클릭·Enter로 다음 페이지로 넘기며, 마지막 대사 이후 자동으로 창을 닫습니다.
// =========================================================================================

#include "NPC/TalkDialogueWidget.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Character/HUDRootWidget.h"
#include "Character/PlayerHUDWidget.h"

// HUDRoot에서 PlayerHUD를 찾아 Visibility를 설정하는 내부 헬퍼입니다.
static void SetPlayerHUDVisibility(UObject* WorldContext, ESlateVisibility InVisibility)
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

// 대화창이 열릴 때 PlayerHUD를 숨깁니다. MinimapWidget은 그대로 유지됩니다.
void UTalkDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetPlayerHUDVisibility(this, ESlateVisibility::Hidden);
}

// 대화창이 닫힐 때 PlayerHUD를 복구합니다.
void UTalkDialogueWidget::NativeDestruct()
{
    SetPlayerHUDVisibility(this, ESlateVisibility::Visible);
    Super::NativeDestruct();
}

// FTalkData를 받아 NPC 이름과 대사 목록을 캐시하고 첫 페이지를 표시합니다.
void UTalkDialogueWidget::StartDialogue(const FTalkData& TalkData)
{
    CurrentDialogues = TalkData.Dialogues;
    CurrentPageIndex = 0;

    if (NPCNameText && !TalkData.NPCName.IsEmpty())
    {
        NPCNameText->SetText(TalkData.NPCName);
    }

    NextDialoguePage();
}

// 다음 대사 페이지를 표시하거나, 모든 대사가 끝나면 창을 닫습니다.
void UTalkDialogueWidget::NextDialoguePage()
{
    if (CurrentPageIndex < CurrentDialogues.Num())
    {
        if (DialogueText)
        {
            // "\n" 이스케이프 시퀀스를 실제 줄바꿈 문자로 변환해 표시합니다.
            FString RawString = CurrentDialogues[CurrentPageIndex].ToString();
            FString FormattedString = RawString.Replace(TEXT("\\n"), TEXT("\n"));
            DialogueText->SetText(FText::FromString(FormattedString));
        }
        CurrentPageIndex++;
        return;
    }

    // 모든 대사가 끝나면 대화창을 닫습니다.
    CloseDialogue();
}

// 대화창을 제거하고 게임 전용 입력 모드와 이동 입력을 복구합니다.
void UTalkDialogueWidget::CloseDialogue()
{
    RemoveFromParent();

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    PC->SetInputMode(FInputModeGameOnly());
    PC->SetShowMouseCursor(false);
    PC->SetIgnoreMoveInput(false);
}
