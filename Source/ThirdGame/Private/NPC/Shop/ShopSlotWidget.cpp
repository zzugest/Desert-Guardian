// =========================================================================================
// ShopSlotWidget.cpp
//
// [파일 역할]
// 상점 그리드에서 판매 상품 한 칸을 표시하는 위젯입니다.
// 마우스 우클릭으로 장바구니에 추가하고, 더블클릭 이벤트를 Handled로 흡수해
// 불필요한 입력이 상위 위젯으로 전파되지 않도록 합니다.
// 아이템이 있을 때는 아이콘·가격 텍스트·툴팁을 표시합니다.
// =========================================================================================

#include "NPC/Shop/ShopSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "NPC/Shop/ShopWidget.h"
#include "Item/ItemTooltipWidget.h"

void UShopSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

// 빈 슬롯 상태로 전환해 아이콘·가격·툴팁을 숨깁니다.
void UShopSlotWidget::SetEmpty()
{
    if (IconImage)  IconImage->SetVisibility(ESlateVisibility::Hidden);
    if (PriceText)  PriceText->SetVisibility(ESlateVisibility::Hidden);
    SetToolTip(nullptr);
}

// 아이템 데이터를 받아 아이콘·가격·툴팁을 설정하고 슬롯을 활성화합니다.
void UShopSlotWidget::SetItemData(FItemData InData)
{
    ItemData = InData;

    if (IconImage)  IconImage->SetVisibility(ESlateVisibility::Visible);
    if (PriceText)  PriceText->SetVisibility(ESlateVisibility::Visible);

    if (PriceText)
    {
        FString PriceStr = FString::Printf(TEXT("%d G"), ItemData.ItemPrice);
        PriceText->SetText(FText::FromString(PriceStr));
    }

    if (IconImage && ItemData.ItemIcon)
    {
        IconImage->SetBrushFromTexture(ItemData.ItemIcon);
    }

    // 툴팁은 최초 1회만 생성하고 이후에는 데이터만 갱신합니다.
    if (TooltipClass)
    {
        if (!CachedTooltip)
        {
            CachedTooltip = CreateWidget<UItemTooltipWidget>(this, TooltipClass);
        }

        if (CachedTooltip)
        {
            CachedTooltip->InitTooltip(ItemData);
            SetToolTip(CachedTooltip);
        }
    }
}

// 장바구니 추가 요청 시 부모 ShopWidget을 참조하기 위해 포인터를 저장합니다.
void UShopSlotWidget::Init(UShopWidget* InShopWidget)
{
    OwnerShopWidget = InShopWidget;
}

// 마우스 버튼 이벤트를 처리합니다.
// 좌클릭: 이벤트를 흡수(상위 전파 방지). 우클릭: 아이템이 있으면 장바구니에 추가합니다.
FReply UShopSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 좌클릭은 이벤트만 흡수하고 별도 동작 없이 처리합니다.
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        return FReply::Handled();
    }

    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        // 빈 슬롯(아이콘 없음)은 장바구니 추가를 무시합니다.
        if (ItemData.ItemIcon == nullptr) return FReply::Unhandled();

        if (OwnerShopWidget)
        {
            OwnerShopWidget->AddToCart(ItemData);
            return FReply::Handled();
        }
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

// 더블클릭 이벤트를 Handled로 흡수해 상위 위젯으로 전파되지 않도록 합니다.
FReply UShopSlotWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}
