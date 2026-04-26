// =========================================================================================
// QuestLogWidget.cpp
//
// [파일 역할]
// 화면 좌측 상단에 항상 표시되어 플레이어의 퀘스트 진행 현황을 실시간으로 알려주는 로그 UI입니다.
// QuestComponent의 OnQuestUIUpdated 델리게이트를 구독해 이벤트 발생 시 텍스트를 갱신합니다.
// 진행 중인 퀘스트가 없으면 배경 Border를 Collapsed로 숨깁니다.
// =========================================================================================

#include "NPC/Quest/QuestLogWidget.h"
#include "Components/RichTextBlock.h"
#include "Components/Border.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/Quest/QuestComponent.h"

// 위젯 생성 시 플레이어의 QuestComponent를 캐시하고 업데이트 이벤트를 바인딩합니다.
void UQuestLogWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!PlayerChar) return;

    CachedQuestComp = Cast<UQuestComponent>(PlayerChar->GetComponentByClass(UQuestComponent::StaticClass()));
    if (!CachedQuestComp) return;

    // 퀘스트 수락·완료·진행도 변경 시 UI를 즉시 갱신하기 위해 델리게이트에 등록합니다.
    CachedQuestComp->OnQuestUIUpdated.AddDynamic(this, &UQuestLogWidget::UpdateQuestLog);

    // 초기 상태를 즉시 반영합니다.
    UpdateQuestLog();
}

// 위젯이 닫힐 때 QuestComponent 델리게이트를 해제합니다.
// 해제하지 않으면 파괴된 위젯의 UpdateQuestLog가 호출되어 크래시가 발생합니다.
void UQuestLogWidget::NativeDestruct()
{
    Super::NativeDestruct();

    if (CachedQuestComp)
    {
        CachedQuestComp->OnQuestUIUpdated.RemoveDynamic(this, &UQuestLogWidget::UpdateQuestLog);
    }
}

// QuestComponent에서 최신 로그 텍스트를 받아 UI에 반영합니다.
// 퀘스트가 없으면 배경 Border를 Collapsed로 처리해 공간을 차지하지 않습니다.
void UQuestLogWidget::UpdateQuestLog()
{
    if (!CachedQuestComp || !QuestListText || !QuestBackground) return;

    FString LogText = CachedQuestComp->GetQuestLogText();
    QuestListText->SetText(FText::FromString(LogText));

    // Collapsed: 공간 차지 없이 완전히 제거 (Visible 시 텍스트 크기에 맞게 자동 복원)
    ESlateVisibility NewVisibility = LogText.IsEmpty()
        ? ESlateVisibility::Collapsed
        : ESlateVisibility::Visible;

    QuestBackground->SetVisibility(NewVisibility);
}
