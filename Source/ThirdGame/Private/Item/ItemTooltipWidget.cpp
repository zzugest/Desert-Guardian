// =========================================================================================
// ItemTooltipWidget.cpp
//
// [파일 역할]
// 인벤토리 슬롯에 마우스를 올렸을 때 표시되는 아이템 툴팁 위젯입니다.
// FItemData를 받아 이름·설명·효과·쿨타임 텍스트를 한 번에 갱신합니다.
// 쿨타임은 ItemEffectClass의 CDO(Class Default Object)에서 읽어
// 새 객체 생성 없이 CooldownTime 값을 참조합니다.
// =========================================================================================

#include "Item/ItemTooltipWidget.h"
#include "Components/TextBlock.h"
#include "Item/ItemEffectBase.h"

// FItemData를 받아 툴팁의 이름·설명·효과·쿨타임 텍스트를 모두 갱신합니다.
void UItemTooltipWidget::InitTooltip(const FItemData& ItemInfo)
{
    // 한국어 아이템 이름을 표시합니다.
    if (ItemNameText)
    {
        ItemNameText->SetText(FText::FromString(ItemInfo.ItemKoreaName));
    }

    // 아이템 설명 텍스트를 표시합니다.
    if (ItemDescText)
    {
        ItemDescText->SetText(FText::FromString(ItemInfo.Description));
    }

    // 아이템 효과 설명 텍스트를 표시합니다.
    if (ItemEffectText)
    {
        ItemEffectText->SetText(FText::FromString(ItemInfo.EffectDescription));
    }

    // 쿨타임은 CDO에서 읽어 표시합니다. 0이면 쿨타임 텍스트를 숨깁니다.
    if (ItemCooldownText)
    {
        if (ItemInfo.ItemEffectClass)
        {
            // CDO에서 읽으면 새 UObject를 생성하지 않고 메모리에 상주한 기본값을 참조합니다.
            UItemEffectBase* CDO = ItemInfo.ItemEffectClass->GetDefaultObject<UItemEffectBase>();
            if (CDO && CDO->CooldownTime > 0.0f)
            {
                FString CooldownStr = FString::Printf(TEXT("(쿨타임 : %.0f초)"), CDO->CooldownTime);
                ItemCooldownText->SetText(FText::FromString(CooldownStr));
                ItemCooldownText->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                // 쿨타임이 0이면 표시하지 않습니다.
                ItemCooldownText->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
        else
        {
            ItemCooldownText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}
