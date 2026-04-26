// =========================================================================================
// QuickSlotWidget.cpp
//
// [���� ���]
// ȭ�� �ϴܿ� ��ġ�Ǵ� ���� �������� �������ϴ� �������� ������/����/����Ű/��Ÿ�� �������� ǥ�� �� �巡�� �� ��� �̺�Ʈ�� �����մϴ�.
// =========================================================================================

#include "QuickSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InventoryItemObject.h"
#include "QuickSlotSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "InventoryDragVisual.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ItemTooltipWidget.h"

// ���޹��� ������ �����Ϳ� ���� ���� �������� ǥ���ϰų� ����ϴ�.
void UQuickSlotWidget::SetItem(const FItemData& NewItem)
{
	if (!IconImage) return;

	if (NewItem.ItemIcon == nullptr)
	{
		IconImage->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	IconImage->SetBrushFromTexture(NewItem.ItemIcon);
	IconImage->SetVisibility(ESlateVisibility::Visible);
}

// ������ �ϴܿ� ǥ�õ� ����Ű ���� �ؽ�Ʈ�� �����մϴ�.
void UQuickSlotWidget::SetKeyNumber(int32 Number)
{
	if (!KeyText) return;

	KeyText->SetText(FText::AsNumber(Number));
}

// ���� ���� �� ����Ű ��ȣ�� �����ϰ� ����ý����� ���� �˸� �̺�Ʈ�� �����մϴ�.
void UQuickSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (KeyText)
	{
		// 9�� �ε����� Ű���� '0'���� ��Ī�ǵ��� ���� �����մϴ�.
		int32 DisplayNum = (QuickSlotIndex + 1) % 10;
		KeyText->SetText(FText::AsNumber(DisplayNum));
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UQuickSlotSubsystem* QuickSubsystem = GI->GetSubsystem<UQuickSlotSubsystem>();
	if (!QuickSubsystem) return;

	QuickSubsystem->OnQuickSlotUpdated.AddDynamic(this, &UQuickSlotWidget::OnQuickSlotSystemUpdated);
	UpdateSlotDisplay();
}

void UQuickSlotWidget::NativeDestruct()
{
	Super::NativeDestruct();

	// 위젯이 파괴될 때 QuickSlotSubsystem 델리게이트 바인딩을 해제합니다.
	// 해제하지 않으면 위젯 재생성 시 중복 등록되어 OnQuickSlotSystemUpdated가 여러 번 호출됩니다.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UQuickSlotSubsystem* QuickSubsystem = GI->GetSubsystem<UQuickSlotSubsystem>())
		{
			QuickSubsystem->OnQuickSlotUpdated.RemoveDynamic(this, &UQuickSlotWidget::OnQuickSlotSystemUpdated);
		}
	}
}



// �ý������κ��� ������ ���� �˸��� �޾��� �� UI ���ΰ�ħ�� �����մϴ�.
void UQuickSlotWidget::OnQuickSlotSystemUpdated()
{
	UpdateSlotDisplay();
}

// ����ý����� �����͸� �о�� ���� ������ �����ܰ� ���� ���� UI�� �ֽ�ȭ�մϴ�.
void UQuickSlotWidget::UpdateSlotDisplay()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UQuickSlotSubsystem* QuickSubsystem = GI->GetSubsystem<UQuickSlotSubsystem>();
	if (!QuickSubsystem) return;

	if (!QuickSubsystem->QuickSlotContent.IsValidIndex(QuickSlotIndex)) return;

	const FItemData& SlotData = QuickSubsystem->QuickSlotContent[QuickSlotIndex];

	// �� ������ ��� ���� �������� �ؽ�Ʈ�� �̹����� �ܻ����� ���� �ʵ��� ��� �����ϰ� �����ϴ�.
	if (SlotData.ItemIcon == nullptr)
	{
		if (IconImage) IconImage->SetColorAndOpacity(FLinearColor::Transparent);
		if (CountText) CountText->SetVisibility(ESlateVisibility::Hidden);

		SetToolTip(nullptr);
		return;
	}

	if (IconImage)
	{
		IconImage->SetBrushFromTexture(SlotData.ItemIcon);
		IconImage->SetColorAndOpacity(FLinearColor::White);
		IconImage->SetVisibility(ESlateVisibility::Visible);
	}

	if (CountText)
	{
		if (SlotData.Quantity > 1)
		{
			CountText->SetText(FText::AsNumber(SlotData.Quantity));
			CountText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			CountText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	if (TooltipClass)
	{
		// 1. ������ ���� �� ���� �� ��������ٸ� ���� ����
		if (!CachedTooltip)
		{
			CachedTooltip = CreateWidget<UItemTooltipWidget>(this, TooltipClass);
		}

		// 2. ������ ���� ������ ������ �ֽ� ������ �о�ֱ�
		if (CachedTooltip)
		{
			CachedTooltip->InitTooltip(SlotData);

			// 3. �𸮾� �������� ���� ���� ���� �ѱ�
			SetToolTip(CachedTooltip);
		}
	}
}

// �� ������ �ش� ���� �������� ���� ���ð�(��Ÿ��)�� ����Ͽ� UI ���̾ ǥ���մϴ�.
void UQuickSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UQuickSlotSubsystem* QuickSubsystem = GI->GetSubsystem<UQuickSlotSubsystem>();
	if (!QuickSubsystem) return;

	if (!QuickSubsystem->QuickSlotContent.IsValidIndex(QuickSlotIndex)) return;

	const FItemData& SlotData = QuickSubsystem->QuickSlotContent[QuickSlotIndex];

	// ��ϵ� �������� ���ų� �Ҹ� ȿ���� ���� �������̸� ��Ÿ�� UI�� ���� �Ҹ��ŵ�ϴ�.
	if (SlotData.ItemIcon == nullptr || !SlotData.ItemEffectClass)
	{
		if (CooldownOverlay) CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
		if (CooldownText) CooldownText->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	float TimeLeft = QuickSubsystem->GetRemainingCooldown(SlotData.ItemEffectClass);

	// ��Ÿ���� ����Ǿ��� �� ���� UI ���̾ ġ��ϴ�.
	if (TimeLeft <= 0.0f)
	{
		if (CooldownOverlay) CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
		if (CooldownText) CooldownText->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	if (CooldownOverlay) CooldownOverlay->SetVisibility(ESlateVisibility::Visible);

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Visible);

		// �Ҽ����� �ø� ó���Ͽ� �������� �������� �� ���� ü���� �����մϴ�.
		int32 IntTime = FMath::CeilToInt(TimeLeft);
		CooldownText->SetText(FText::AsNumber(IntTime));
	}
}

// ������ Ŭ������ �� �巡�� �� ��� ���� ���θ� �Ǻ��մϴ�.
FReply UQuickSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// ��Ŭ���� �ƴϸ� �Ϲ����� ���� �׼� ó���� �������� �ѱ�ϴ�.
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

// �巡�װ� �����Ǿ��� �� ���콺�� ����ٴ� �ð� ��ҿ� ������ ����(Payload)�� �����մϴ�.
void UQuickSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UQuickSlotSubsystem* QuickSubsystem = GI->GetSubsystem<UQuickSlotSubsystem>();
	if (!QuickSubsystem || !QuickSubsystem->QuickSlotContent.IsValidIndex(QuickSlotIndex)) return;

	// ����ִ� ������ ����ȭ���̳� �ٸ� ĭ���� ��� �����ϴ� ���׸� �����մϴ�.
	if (QuickSubsystem->QuickSlotContent[QuickSlotIndex].ItemIcon == nullptr) return;

	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();

	UQuickSlotDragPayload* PayloadObj = NewObject<UQuickSlotDragPayload>();
	PayloadObj->SourceSlotIndex = QuickSlotIndex;
	DragOp->Payload = PayloadObj;

	if (DragVisualClass)
	{
		UInventoryDragVisual* VisualWidget = CreateWidget<UInventoryDragVisual>(this, DragVisualClass);
		if (VisualWidget)
		{
			UTexture2D* IconToDraw = QuickSubsystem->QuickSlotContent[QuickSlotIndex].ItemIcon;
			VisualWidget->SetDragIcon(IconToDraw);
			DragOp->DefaultDragVisual = VisualWidget;
		}
	}
	else
	{
		DragOp->DefaultDragVisual = this;
	}

	DragOp->Pivot = EDragPivot::CenterCenter;
	OutOperation = DragOp;
}

// �巡�� ���� ��Ҹ� �� ���� ���� ������ ���� ��� �� ��ü ó���� �����մϴ�.
bool UQuickSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UQuickSlotSubsystem* QuickSubsystem = GI->GetSubsystem<UQuickSlotSubsystem>();
	if (!QuickSubsystem) return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// �κ��丮 ȭ�鿡�� ����� ���� ��� ���ο� ��ü�� �����Ͽ� �����ϴ�.
	UInventoryItemObject* InvenPayload = Cast<UInventoryItemObject>(InOperation->Payload);
	if (InvenPayload)
	{
		QuickSubsystem->RegisterQuickSlot(QuickSlotIndex, InvenPayload->ItemData);
		return true;
	}

	// �ٸ� ������ ��ȣ���� ����� ���� ��� �� ��� ���� ��ġ�� ���� �±�ȯ ó���մϴ�.
	UQuickSlotDragPayload* QuickSlotPayload = Cast<UQuickSlotDragPayload>(InOperation->Payload);
	if (QuickSlotPayload)
	{
		QuickSubsystem->SwapQuickSlots(QuickSlotPayload->SourceSlotIndex, QuickSlotIndex);
		return true;
	}

	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
