// =========================================================================================
// QuestNPC.cpp
//
// [파일 역할]
// 플레이어와 상호작용하여 퀘스트 수락·진행 대화·완료 보상 처리를 담당하는 NPC 클래스입니다.
// 머리 위 마커(!, ?, ...)로 퀘스트 상태를 실시간으로 표시하며,
// 플레이어의 퀘스트 상태 변화 시 OnQuestUIUpdated 델리게이트를 통해 마커를 자동 갱신합니다.
// =========================================================================================

#include "NPC/Quest/QuestNPC.h"
#include "MyCharacter.h"
#include "NPC/Quest/QuestComponent.h"
#include "Engine/Engine.h"
#include "NPC/Quest/QuestDialogueWidget.h"
#include "Blueprint/UserWidget.h"
#include "UISubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/SlateColor.h"
#include "NPC/Quest/QuestMarkerWidget.h"

// NPC 머리 위 퀘스트 상태 마커(3D 위젯 컴포넌트)를 초기화합니다.
AQuestNPC::AQuestNPC()
{
    QuestMarkerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("QuestMarkerWidget"));
    QuestMarkerWidget->SetupAttachment(GetRootComponent());

    // Screen 공간 설정: 카메라 회전과 무관하게 항상 화면을 향하도록 합니다.
    QuestMarkerWidget->SetWidgetSpace(EWidgetSpace::Screen);
    QuestMarkerWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
    QuestMarkerWidget->SetDrawSize(FVector2D(100.f, 100.f));
}

// 초기 마커 상태를 설정하고 플레이어의 퀘스트 업데이트 이벤트를 구독합니다.
void AQuestNPC::BeginPlay()
{
    Super::BeginPlay();

    AMyCharacter* PlayerChar = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerChar || !PlayerChar->QuestComponent) return;

    UpdateQuestMarker(PlayerChar);

    // 퀘스트 수락·진행·완료 시 마커를 자동으로 갱신합니다.
    PlayerChar->QuestComponent->OnQuestUIUpdated.AddDynamic(this, &AQuestNPC::RefreshMarker);
}

// NPC가 파괴될 때 델리게이트를 해제합니다.
// 해제하지 않으면 파괴된 NPC의 RefreshMarker가 퀘스트 진행 중 호출됩니다.
void AQuestNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    AMyCharacter* PlayerChar = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (PlayerChar && PlayerChar->QuestComponent)
    {
        PlayerChar->QuestComponent->OnQuestUIUpdated.RemoveDynamic(this, &AQuestNPC::RefreshMarker);
    }
}

// 델리게이트 콜백: 퀘스트 상태가 변경되면 마커를 최신 상태로 갱신합니다.
void AQuestNPC::RefreshMarker()
{
    AMyCharacter* PlayerChar = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    UpdateQuestMarker(PlayerChar);
}

// 플레이어가 NPC와 상호작용할 때 퀘스트 상태에 맞는 대화창을 엽니다.
// 우선순위: 완료 가능 > 새 퀘스트 수락 > 진행 중 안내
void AQuestNPC::InteractWithPlayer(AMyCharacter* PlayerCharacter)
{
    Super::InteractWithPlayer(PlayerCharacter);

    if (!PlayerCharacter || !PlayerCharacter->QuestComponent) return;
    if (!DialogueWidgetClass || !QuestDataTable) return;

    UQuestComponent* QuestComp = PlayerCharacter->QuestComponent;

    FName TargetQuestID = NAME_None;
    int32 InteractionType = 0; // 1: 완료, 2: 수락 가능, 3: 진행 중

    // 우선순위 1: 완료 가능한 퀘스트 확인
    for (FName QuestID : AvailableQuests)
    {
        if (QuestComp->IsQuestReadyToComplete(QuestID))
        {
            TargetQuestID = QuestID;
            InteractionType = 1;
            break;
        }
    }

    // 우선순위 2: 새로 수락 가능한 퀘스트 확인 (완료 가능 상태가 없을 때)
    if (InteractionType == 0)
    {
        for (FName QuestID : AvailableQuests)
        {
            if (QuestComp->CanAcceptQuest(QuestID))
            {
                TargetQuestID = QuestID;
                InteractionType = 2;
                break;
            }
        }
    }

    // 우선순위 3: 진행 중인 퀘스트 안내 대화
    if (InteractionType == 0)
    {
        for (FName QuestID : AvailableQuests)
        {
            if (QuestComp->IsQuestActive(QuestID))
            {
                TargetQuestID = QuestID;
                InteractionType = 3;
                break;
            }
        }
    }

    // 해당되는 퀘스트 상태가 없으면 대화창을 열지 않습니다.
    if (InteractionType == 0) return;

    FQuestData* TargetQuestData = QuestDataTable->FindRow<FQuestData>(TargetQuestID, TEXT("QuestDialogue"));
    if (!TargetQuestData) return;

    // 대화창을 생성하고 공통 상태(이동 차단·마우스 활성화)를 설정합니다.
    APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
    UQuestDialogueWidget* DialogueUI = CreateWidget<UQuestDialogueWidget>(PC, DialogueWidgetClass);
    if (!DialogueUI) return;

    DialogueUI->AddToViewport();
    SetupDialogueState(PlayerCharacter, DialogueUI);

    // 판별된 상호작용 타입에 맞는 대화 흐름을 시작합니다.
    if (InteractionType == 1)
    {
        DialogueUI->StartCompletionDialogue(TargetQuestID, *TargetQuestData, QuestComp);
    }
    else if (InteractionType == 2)
    {
        DialogueUI->StartDialogue(TargetQuestID, *TargetQuestData, QuestComp);
    }
    else if (InteractionType == 3)
    {
        DialogueUI->StartNoticeDialogue(TargetQuestData->InProgressDialogue);
    }
}

// 퀘스트 목록 우선순위(완료→수락→진행 중)에 따라 마커 상태 번호를 결정하고 UI에 반영합니다.
void AQuestNPC::UpdateQuestMarker(AMyCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !PlayerCharacter->QuestComponent) return;

    UQuestComponent* QuestComp = PlayerCharacter->QuestComponent;
    int32 MarkerState = 0; // 0: 숨김, 1: 완료 가능(?), 2: 수락 가능(!), 3: 진행 중(...)

    for (FName QuestID : AvailableQuests)
    {
        if (QuestComp->IsQuestReadyToComplete(QuestID))
        {
            MarkerState = 1;
            break;
        }
    }

    if (MarkerState == 0)
    {
        for (FName QuestID : AvailableQuests)
        {
            if (QuestComp->CanAcceptQuest(QuestID))
            {
                MarkerState = 2;
                break;
            }
        }
    }

    if (MarkerState == 0)
    {
        for (FName QuestID : AvailableQuests)
        {
            if (QuestComp->IsQuestActive(QuestID))
            {
                MarkerState = 3;
                break;
            }
        }
    }

    UpdateMarkerUI(MarkerState);
}

// 마커 상태 번호에 따라 텍스트와 색상을 설정합니다.
// 0이면 마커 컴포넌트 전체를 숨깁니다.
void AQuestNPC::UpdateMarkerUI(int32 MarkerState)
{
    if (!QuestMarkerWidget) return;

    if (MarkerState == 0)
    {
        QuestMarkerWidget->SetVisibility(false);
        return;
    }

    QuestMarkerWidget->SetVisibility(true);

    UQuestMarkerWidget* MarkerObj = Cast<UQuestMarkerWidget>(QuestMarkerWidget->GetUserWidgetObject());
    if (MarkerObj && MarkerObj->MarkerText)
    {
        switch (MarkerState)
        {
        case 1: // 퀘스트 완료 가능 (노란색 ?)
            MarkerObj->MarkerText->SetText(FText::FromString(TEXT("?")));
            MarkerObj->MarkerText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
            break;

        case 2: // 퀘스트 수락 가능 (노란색 !)
            MarkerObj->MarkerText->SetText(FText::FromString(TEXT("!")));
            MarkerObj->MarkerText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
            break;

        case 3: // 퀘스트 진행 중 (회색 ...)
            MarkerObj->MarkerText->SetText(FText::FromString(TEXT("...")));
            MarkerObj->MarkerText->SetColorAndOpacity(FSlateColor(FLinearColor::Gray));
            break;
        }
    }
}
