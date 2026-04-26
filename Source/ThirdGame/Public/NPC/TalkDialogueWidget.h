// =========================================================================================
// TalkDialogueWidget.h
//
// [역할 설명]
// 일반 NPC와 대화할 때 화면에 대사를 표시하고, 대화창이 열릴 때/닫힐 때
// 플레이어 HUD를 숨기고 복구하는 대화 UI 위젯 클래스입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TalkData.h"
#include "TalkDialogueWidget.generated.h"

class UTextBlock;

UCLASS()
class THIRDGAME_API UTalkDialogueWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    // UI 생성 시 PlayerHUD를 숨깁니다.
    virtual void NativeConstruct() override;

    // 대화창이 닫힐 때 PlayerHUD를 복구합니다.
    virtual void NativeDestruct() override;

public:
    // NPC의 대화 텍스트가 표시되는 위젯입니다.
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DialogueText;

    // 대화창 상단에 표시될 NPC의 이름 위젯입니다. (선택 사항)
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* NPCNameText;

    // 대본 데이터를 받아 대화 흐름을 초기화하고 첫 페이지를 화면에 표시합니다.
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(const FTalkData& TalkData);

    // 현재 페이지 다음으로 넘어가거나, 모든 페이지를 읽었으면 대화창을 닫습니다.
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NextDialoguePage();

private:
    void CloseDialogue();

    TArray<FText> CurrentDialogues;
    int32 CurrentPageIndex = 0;
};
