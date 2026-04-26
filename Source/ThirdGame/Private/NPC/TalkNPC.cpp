// =========================================================================================
// TalkNPC.cpp
//
// [파일 역할]
// 퀘스트 없이 단순 대화만 제공하는 NPC 클래스입니다.
// 플레이어가 상호작용하면 DialogueWidgetClass로 대화창을 생성하고,
// DataTable에서 읽어온 FTalkData를 TalkDialogueWidget에 넘겨 대사를 표시합니다.
// 입력 모드 전환 등 공통 설정은 BaseNPC::SetupDialogueState에 위임합니다.
// =========================================================================================

#include "NPC/TalkNPC.h"
#include "Blueprint/UserWidget.h"
#include "MyCharacter.h"
#include "NPC/TalkDialogueWidget.h"

ATalkNPC::ATalkNPC()
{
    // 추가 컴포넌트 없음. 기반 클래스(Mesh, Box, Prompt Widget)를 그대로 사용합니다.
}

// 플레이어가 상호작용할 때 대화창을 생성하고 DataTable에서 읽어온 대사를 시작합니다.
void ATalkNPC::InteractWithPlayer(AMyCharacter* PlayerCharacter)
{
    Super::InteractWithPlayer(PlayerCharacter);

    if (!PlayerCharacter || !DialogueWidgetClass) return;

    UUserWidget* WidgetObj = CreateWidget<UUserWidget>(GetWorld(), DialogueWidgetClass);
    if (!WidgetObj) return;

    WidgetObj->AddToViewport();

    // 이동 차단·마우스 활성화 등 대화 공통 상태를 설정합니다.
    SetupDialogueState(PlayerCharacter, WidgetObj);

    // DataTable에서 이 NPC의 대사 행을 찾아 TalkDialogueWidget에 전달합니다.
    UTalkDialogueWidget* TalkWidget = Cast<UTalkDialogueWidget>(WidgetObj);
    if (TalkWidget && TalkDataTable && !NPCTalkRowName.IsNone())
    {
        FTalkData* RowData = TalkDataTable->FindRow<FTalkData>(NPCTalkRowName, TEXT("TalkNPC_Lookup"));
        if (RowData)
        {
            TalkWidget->StartDialogue(*RowData);
        }
    }
}
