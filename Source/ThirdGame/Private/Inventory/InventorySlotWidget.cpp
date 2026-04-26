// =========================================================================================
// InventorySlotWidget.cpp
//
// [파일 역할]
// 인벤토리 화면 내 개별 슬롯(칸) 하나를 표현하는 UI 위젯 클래스입니다.
// TileView로부터 데이터를 받아 아이콘/수량을 표시하고,
// 드래그 시작·드롭 이벤트를 처리해 인벤토리 서브시스템에 위치 교환을 요청합니다.
// 아이템이 있는 슬롯에 마우스를 올리면 툴팁 위젯을 표시합니다.
// =========================================================================================

#include "Inventory/InventorySlotWidget.h"
#include "Components/Image.h"
#include "Inventory/InventoryItemObject.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/TextBlock.h"
#include "Inventory/InventoryDragVisual.h"
#include "Inventory/InventorySubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Item/ItemTooltipWidget.h"

// 좌클릭 시 아이템이 있는 슬롯이면 드래그 감지 루틴을 시작합니다.
FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 좌클릭이 아닌 경우 기본 처리를 넘겨줍니다.
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	UInventoryItemObject* ItemObj  = GetListItem<UInventoryItemObject>();
	bool bIsRealItem = (ItemObj != nullptr) && (ItemObj->ItemData.ItemIcon != nullptr);

	if (!bIsRealItem)
	{
		// 빈 슬롯 클릭 시 드래그 이벤트가 발생하지 않도록 Handled만 반환합니다.
		return FReply::Handled();
	}

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

// 드래그가 감지되면 아이템 데이터를 Payload로 담은 DragDropOperation을 생성합니다.
void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();

	UInventoryItemObject* ItemObj = GetListItem<UInventoryItemObject>();
	DragOp->Payload = ItemObj;

	// 드래그 중 커서 옆에 표시할 아이콘 비주얼 위젯을 생성합니다.
	if (DragVisualClass)
	{
		UInventoryDragVisual* VisualWidget = CreateWidget<UInventoryDragVisual>(this, DragVisualClass);
		if (VisualWidget)
		{
			if (ItemObj) VisualWidget->SetDragIcon(ItemObj->ItemData.ItemIcon);

			DragOp->DefaultDragVisual = VisualWidget;
		}
	}

	OutOperation = DragOp;
}

// TileView가 슬롯에 데이터를 할당할 때 호출되어 SetItem으로 UI를 갱신합니다.
void UInventorySlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	UInventoryItemObject* ItemObj = Cast<UInventoryItemObject>(ListItemObject);
	if (!ItemObj) return;

	SetItem(&ItemObj->ItemData);
}

// 아이템 데이터를 읽어 아이콘·수량 텍스트·툴팁을 갱신합니다. 빈 슬롯이면 모두 숨깁니다.
void UInventorySlotWidget::SetItem(const FItemData* NewItem)
{
	if (!IconImage || !AmountText) return;

	if (NewItem && NewItem->ItemIcon != nullptr)
	{
		IconImage->SetBrushFromTexture(NewItem->ItemIcon);
		IconImage->SetVisibility(ESlateVisibility::Visible);

		// 수량이 2 이상일 때만 수량 텍스트를 표시합니다.
		if (NewItem->Quantity > 1)
		{
			AmountText->SetText(FText::AsNumber(NewItem->Quantity));
			AmountText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			AmountText->SetVisibility(ESlateVisibility::Hidden);
		}

		if (TooltipClass)
		{
			// 툴팁 위젯은 최초 1회만 생성하고 이후에는 데이터만 갱신합니다.
			if (!CachedTooltip)
			{
				CachedTooltip = CreateWidget<UItemTooltipWidget>(this, TooltipClass);
			}

			if (CachedTooltip)
			{
				CachedTooltip->InitTooltip(*NewItem);
				SetToolTip(CachedTooltip);
			}
		}
	}
	else
	{
		// 빈 슬롯이면 아이콘·수량·툴팁을 모두 숨깁니다.
		IconImage->SetVisibility(ESlateVisibility::Hidden);
		AmountText->SetVisibility(ESlateVisibility::Hidden);
		SetToolTip(nullptr);
	}
}

// 드래그한 아이템을 이 슬롯에 드롭하면 서브시스템에 두 인덱스의 위치 교환을 요청합니다.
bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UInventoryItemObject* PayloadObj = Cast<UInventoryItemObject>(InOperation->Payload);
	if (!PayloadObj) return false;

	UInventoryItemObject* MyItemObj = GetListItem<UInventoryItemObject>();
	if (!MyItemObj) return false;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return false;

	UInventorySubsystem* InvenSubsystem = GI->GetSubsystem<UInventorySubsystem>();
	if (!InvenSubsystem) return false;

	// UI가 아닌 서브시스템에서 실제 데이터 배열을 교환하고 OnInventoryUpdated를 브로드캐스트합니다.
	InvenSubsystem->SwapItems(PayloadObj->Index, MyItemObj->Index);

	return true;
}
