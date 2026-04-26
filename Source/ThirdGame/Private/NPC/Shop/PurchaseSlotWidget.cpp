// =========================================================================================
// PurchaseSlotWidget.cpp
//
// [파일 역할]
// 상점 장바구니(Cart) 목록에 표시되는 개별 슬롯 위젯입니다.
// 담긴 아이템 아이콘과 수량(x N)을 표시하며, 빈 슬롯이면 아이콘·수량을 모두 숨깁니다.
// =========================================================================================

#include "NPC/Shop/PurchaseSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

// 빈 슬롯 상태로 전환해 아이콘과 수량 텍스트를 숨깁니다.
void UPurchaseSlotWidget::SetEmpty()
{
    if (IconImage)  IconImage->SetVisibility(ESlateVisibility::Hidden);
    if (CountText)  CountText->SetVisibility(ESlateVisibility::Hidden);
}

// 장바구니 데이터를 받아 아이콘과 수량 텍스트를 갱신합니다.
void UPurchaseSlotWidget::SetCartData(FCartItem InCartItem)
{
    if (!IconImage || !CountText) return;

    IconImage->SetVisibility(ESlateVisibility::Visible);
    CountText->SetVisibility(ESlateVisibility::Visible);

    IconImage->SetBrushFromTexture(InCartItem.Data.ItemIcon);

    // "x N" 형식으로 수량을 표시해 플레이어가 몇 개를 담았는지 알 수 있게 합니다.
    FString CountStr = FString::Printf(TEXT("x %d"), InCartItem.Quantity);
    CountText->SetText(FText::FromString(CountStr));
}
