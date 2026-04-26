// =========================================================================================
// QuestRewardSlotWidget.cpp
//
// [파일 역할]
// 퀘스트 완료/수락 대화창에서 보상 목록 한 칸을 표시하는 위젯입니다.
// 아이콘 이미지와 수량 텍스트를 세팅하며, 골드와 아이템 보상 모두 같은 슬롯을 재사용합니다.
// =========================================================================================

#include "NPC/Quest/QuestRewardSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

// 아이콘 텍스처와 수량을 받아 슬롯 UI를 갱신합니다.
void UQuestRewardSlotWidget::SetupSlot(UTexture2D* Icon, FString Name, int32 Quantity)
{
    if (RewardIcon && Icon)
    {
        RewardIcon->SetBrushFromTexture(Icon);
    }

    if (RewardText)
    {
        // 수량만 표시합니다 (예: "5")
        FString DisplayString = FString::Printf(TEXT("%d"), Quantity);
        RewardText->SetText(FText::FromString(DisplayString));
    }
}
