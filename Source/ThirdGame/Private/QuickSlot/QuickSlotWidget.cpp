// =========================================================================================
// QuickSlotWidget.cpp
//
// [파일 역할]
// 화면 하단에 위치하는 퀵슬롯 위젯을 구성하며 아이콘/개수/단축키/쿨타임 정보를 표시 및 드래그 앤 드롭 관련 이벤트를 처리합니다.
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

// 전달받은 아이템의 아이콘을 해당 슬롯의 이미지 위젯에 표시하거나 숨깁니다.
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

// 슬롯 하단에 표시된 단축키 번호 텍스트를 설정합니다.
void UQuickSlotWidget::SetKeyNumber(int32 Number)
{
	if (!KeyText) return;

	KeyText->SetText(FText::AsNumber(Number));
}

// 위젯 생성 시 단축키 번호를 설정하고 서브시스템의 갱신 알림 이벤트를 바인딩합니다.
void UQuickSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (KeyText)
	{
		// 9번 인덱스의 키번호가 '0'으로 대칭되도록 나머지 연산을 처리합니다.
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



// 서브시스템으로부터 퀵슬롯 갱신 알림을 받았을 때 UI 표시 갱신을 처리합니다.
void UQuickSlotWidget::OnQuickSlotSystemUpdated()
{
	UpdateSlotDisplay();
}

// 서브시스템의 데이터를 읽어서 아이템 아이콘과 개수 및 쿨타임 표시 UI를 최신화합니다.
void UQuickSlotWidget::UpdateSlotDisplay()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UQuickSlotSubsystem* QuickSubsystem = GI->GetSubsystem<UQuickSlotSubsystem>();
	if (!QuickSubsystem) return;

	if (!QuickSubsystem->QuickSlotContent.IsValidIndex(QuickSlotIndex)) return;

	const FItemData& SlotData = QuickSubsystem->QuickSlotContent[QuickSlotIndex];

	// 빈 슬롯의 경우 아이콘과 텍스트를 투명하게 하여 아무것도 표시되지 않도록 처리합니다.
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
		// 1. 캐시된 툴팁 위젯이 없을 경우 새로 생성
		if (!CachedTooltip)
		{
			CachedTooltip = CreateWidget<UItemTooltipWidget>(this, TooltipClass);
		}

		// 2. 캐시된 툴팁에 최신 슬롯 데이터를 읽어오기
		if (CachedTooltip)
		{
			CachedTooltip->InitTooltip(SlotData);

			// 3. 슬레이트 최상위에 툴팁 팝업 위젯 넘기기
			SetToolTip(CachedTooltip);
		}
	}
}

// 매 프레임 해당 슬롯 아이템의 남은 쿨타임을 확인하여 UI 오버레이를 표시합니다.
void UQuickSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UQuickSlotSubsystem* QuickSubsystem = GI->GetSubsystem<UQuickSlotSubsystem>();
	if (!QuickSubsystem) return;

	if (!QuickSubsystem->QuickSlotContent.IsValidIndex(QuickSlotIndex)) return;

	const FItemData& SlotData = QuickSubsystem->QuickSlotContent[QuickSlotIndex];

	// 등록된 아이템이 없거나 소모 효과가 없는 아이템이면 쿨타임 UI를 숨깁니다.
	if (SlotData.ItemIcon == nullptr || !SlotData.ItemEffectClass)
	{
		if (CooldownOverlay) CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
		if (CooldownText) CooldownText->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	float TimeLeft = QuickSubsystem->GetRemainingCooldown(SlotData.ItemEffectClass);

	// 쿨타임이 완료되었을 때 관련 UI 오버레이를 숨깁니다.
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

		// 소수점을 올림 처리하여 표시되는 숫자가 자연스럽게 보이도록 처리합니다.
		int32 IntTime = FMath::CeilToInt(TimeLeft);
		CooldownText->SetText(FText::AsNumber(IntTime));
	}
}

// 마우스 클릭 시 해당 위젯의 드래그 앤 드롭 여부를 판별합니다.
FReply UQuickSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 좌클릭이 아니면 일반적인 기본 액션 처리에 이벤트를 넘깁니다.
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

// 드래그가 감지되었을 때 마우스를 끌어다 놓는 시각 요소와 페이로드(Payload)를 설정합니다.
void UQuickSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UQuickSlotSubsystem* QuickSubsystem = GI->GetSubsystem<UQuickSlotSubsystem>();
	if (!QuickSubsystem || !QuickSubsystem->QuickSlotContent.IsValidIndex(QuickSlotIndex)) return;

	// 비어있는 슬롯에서 드래그나 다른 칸으로의 이동을 방지하는 가드를 설정합니다.
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

// 드래그 앤 드롭 완료 시 이 위젯 위에 놓인 경우의 출처별 처리를 담당합니다.
bool UQuickSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UQuickSlotSubsystem* QuickSubsystem = GI->GetSubsystem<UQuickSlotSubsystem>();
	if (!QuickSubsystem) return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// 인벤토리 화면에서 드래그된 아이템의 경우 해당 슬롯에 새로운 항목을 등록합니다.
	UInventoryItemObject* InvenPayload = Cast<UInventoryItemObject>(InOperation->Payload);
	if (InvenPayload)
	{
		QuickSubsystem->RegisterQuickSlot(QuickSlotIndex, InvenPayload->ItemData);
		return true;
	}

	// 다른 퀵슬롯을 드래그하여 놓은 경우 두 슬롯 위치를 서로 교환 처리합니다.
	UQuickSlotDragPayload* QuickSlotPayload = Cast<UQuickSlotDragPayload>(InOperation->Payload);
	if (QuickSlotPayload)
	{
		QuickSubsystem->SwapQuickSlots(QuickSlotPayload->SourceSlotIndex, QuickSlotIndex);
		return true;
	}

	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
