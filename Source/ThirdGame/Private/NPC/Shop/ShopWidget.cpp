// =========================================================================================
// ShopWidget.cpp
//
// [파일 역할]
// 전체 상점 UI를 담당하는 루트 위젯입니다.
// 판매 슬롯 그리드 표시, 장바구니 추가·표시, 합산 금액 계산, 최종 구매 처리를 담당합니다.
// UISubsystem에 열림·닫힘을 보고해 마우스 커서와 입력 모드를 자동 전환합니다.
// =========================================================================================

#include "NPC/Shop/ShopWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "NPC/Shop/ShopSlotWidget.h"
#include "NPC/Shop/PurchaseSlotWidget.h"
#include "MoneyComponent.h"
#include "Inventory/InventorySubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "MyCharacter.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "UISubsystem.h"
#include "GlobalUI/WarningSubsystem.h"

UShopWidget::UShopWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    // 상점이 열리면 이동 입력을 차단합니다.
    bShouldBlockMoveInput = true;
}

// 버튼 이벤트를 바인딩하고 UISubsystem에 상점이 열렸음을 알립니다.
void UShopWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (FinalBuyButton)
    {
        FinalBuyButton->OnClicked.AddDynamic(this, &UShopWidget::OnClickFinalBuy);
    }
    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UShopWidget::OnClickClose);
    }

    UGameInstance* GI = GetGameInstance();
    if (GI)
    {
        if (UUISubsystem* UISys = GI->GetSubsystem<UUISubsystem>())
        {
            // UISubsystem이 열린 위젯 목록을 관리해 마우스 커서와 입력 모드를 자동 전환합니다.
            UISys->ReportUIOpened(this);
        }
    }
}

// UISubsystem에 상점이 닫혔음을 알려 입력 모드를 복구합니다.
void UShopWidget::NativeDestruct()
{
    Super::NativeDestruct();

    UGameInstance* GI = GetGameInstance();
    if (GI)
    {
        if (UUISubsystem* UISys = GI->GetSubsystem<UUISubsystem>())
        {
            UISys->ReportUIClosed(this);
        }
    }
}

// NPC가 판매하는 아이템 목록을 받아 슬롯 그리드를 채웁니다.
// 슬롯 수는 항상 MaxSlots(20)으로 고정하며 빈 슬롯은 SetEmpty()로 처리합니다.
void UShopWidget::InitShop(const TArray<FItemData>& SaleItems)
{
    if (!ItemGrid || !ShopSlotClass) return;

    ItemGrid->ClearChildren();

    const int32 MaxSlots = 20;

    for (int32 i = 0; i < MaxSlots; i++)
    {
        UShopSlotWidget* NewSlot = CreateWidget<UShopSlotWidget>(this, ShopSlotClass);
        if (!NewSlot) continue;

        // 장바구니 추가 시 부모 ShopWidget을 참조할 수 있도록 포인터를 전달합니다.
        NewSlot->Init(this);

        if (i < SaleItems.Num())
        {
            NewSlot->SetItemData(SaleItems[i]);
        }
        else
        {
            NewSlot->SetEmpty();
        }

        // 5열 균등 그리드로 배치합니다 (행 = i/5, 열 = i%5).
        ItemGrid->AddChildToUniformGrid(NewSlot, i / 5, i % 5);
    }
}

// 장바구니에 아이템을 추가합니다.
// 같은 이름의 아이템이 이미 있으면 수량만 증가시킵니다. 최대 5종까지 담을 수 있습니다.
void UShopWidget::AddToCart(FItemData ItemToAdd)
{
    for (FCartItem& CartItem : CartItems)
    {
        if (CartItem.Data.ItemName == ItemToAdd.ItemName)
        {
            CartItem.Quantity++;
            UpdateCartUI();
            UpdateTotalPrice();
            return;
        }
    }

    // 장바구니 최대 종류(5)를 초과하면 추가하지 않습니다.
    if (CartItems.Num() >= 5) return;

    FCartItem NewItem;
    NewItem.Data     = ItemToAdd;
    NewItem.Quantity = 1;
    CartItems.Add(NewItem);

    UpdateCartUI();
    UpdateTotalPrice();
}

// CartItems 배열 내용을 화면 장바구니 슬롯에 반영합니다.
void UShopWidget::UpdateCartUI()
{
    if (!CartGrid || !PurchaseSlotClass) return;

    CartGrid->ClearChildren();

    for (int32 i = 0; i < 5; i++)
    {
        UPurchaseSlotWidget* NewSlot = CreateWidget<UPurchaseSlotWidget>(this, PurchaseSlotClass);
        if (!NewSlot) continue;

        if (i < CartItems.Num())
        {
            NewSlot->SetCartData(CartItems[i]);
        }
        else
        {
            NewSlot->SetEmpty();
        }

        CartGrid->AddChildToHorizontalBox(NewSlot);
    }
}

// 장바구니 내 모든 아이템의 합산 금액을 계산해 UI 텍스트에 표시합니다.
void UShopWidget::UpdateTotalPrice()
{
    if (!TotalPriceText) return;

    int32 Total = 0;
    for (const FCartItem& Item : CartItems)
    {
        Total += (Item.Data.ItemPrice * Item.Quantity);
    }

    TotalPriceText->SetText(FText::AsNumber(Total));
}

// 최종 구매 버튼: 잔액을 확인하고 통과하면 아이템을 인벤토리에 추가한 뒤 장바구니를 초기화합니다.
void UShopWidget::OnClickFinalBuy()
{
    if (CartItems.IsEmpty()) return;

    APawn* PlayerPawn = GetOwningPlayerPawn();
    if (!PlayerPawn) return;

    UMoneyComponent* MoneyComp = PlayerPawn->FindComponentByClass<UMoneyComponent>();
    if (!MoneyComp) return;

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UInventorySubsystem* InvenSys = GI->GetSubsystem<UInventorySubsystem>();
    if (!InvenSys) return;

    // 합산 금액 계산
    int32 TotalCost = 0;
    for (const FCartItem& Item : CartItems)
    {
        TotalCost += (Item.Data.ItemPrice * Item.Quantity);
    }

    // 잔액 부족 시 경고 메시지를 표시하고 구매를 취소합니다.
    if (!MoneyComp->TryBuyItem(TotalCost))
    {
        UWarningSubsystem* WarningSys = GI->GetSubsystem<UWarningSubsystem>();
        if (WarningSys)
        {
            FText WarningText = FText::FromStringTable(
                TEXT("/Game/character/ST_WarningMessages.ST_WarningMessages"),
                TEXT("Err_NotEnoughMoney")
            );
            WarningSys->ShowWarning(WarningText);
        }
        return;
    }

    // 각 아이템을 수량만큼 서버 RPC를 통해 서버 권위 인벤토리에 추가합니다.
    AMyCharacter* MyChar = Cast<AMyCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (MyChar && MyChar->InventoryComp)
    {
        for (const FCartItem& Item : CartItems)
        {
            for (int32 i = 0; i < Item.Quantity; i++)
            {
                MyChar->ServerBuyItem(Item.Data);
            }
        }
    }

    // 구매 완료 후 장바구니와 UI를 초기화합니다.
    CartItems.Empty();
    UpdateCartUI();
    UpdateTotalPrice();
}

// 닫기 버튼: 상점 위젯을 화면에서 제거합니다. UISubsystem 해제는 NativeDestruct에서 처리됩니다.
void UShopWidget::OnClickClose()
{
    RemoveFromParent();
}
