// =========================================================================================
// InventorySlotWidget.h
//
// [역할 요약]
// 인벤토리 화면 내 개별 슬롯 1칸을 담당하는 UI 위젯 클래스 헤더입니다.
// 타일 뷰로부터 아이템 정보를 전달받아 아이콘과 수량을 표시하고, 마우스 드래그 앤 드롭 슬롯 이동 기능을 지원합니다.
// =========================================================================================

#pragma once


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "Blueprint/IUserObjectListEntry.h" 
#include "InventorySlotWidget.generated.h"


class UItemTooltipWidget;

UCLASS()
class THIRDGAME_API UInventorySlotWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:

	// 리스트 뷰에서 이 위젯 슬롯에 특정 아이템 데이터(오브젝트)를 동적으로 할당할 때 호출되는 초기화 콜백입니다.
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	// 전달받은 구조체 데이터를 바탕으로 슬롯의 아이콘과 수량 텍스트 시각 요소를 갱신합니다.
	void SetItem(const FItemData* NewItem);

protected:

	// 아이템의 외형(이미지 텍스처)을 화면에 띄우는 이미지 위젯입니다.
	UPROPERTY(meta = (BindWidget))
	class UImage* IconImage;

	// 아이템의 현재 겹침(스택) 보유 수량을 표기하는 텍스트 위젯입니다.
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AmountText;

	// 슬롯을 클릭했을 때 마우스 동작을 판단하여 드래그 시작 여부를 감지합니다.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 실제 마우스 드래그가 진행되었을 때, 아이템 정보를 담은 드래그 전용 운반체(Payload)를 생성합니다.
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	// 드래그하는 동안 마우스 커서를 따라다닐 잔상(비주얼 위젯)을 생성하기 위한 클래스입니다.
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop")
	TSubclassOf<UUserWidget> DragVisualClass;

	// 다른 슬롯에서 드래그해 온 아이템이 이 슬롯 위에 놓여졌을 때(드롭), 위치 교환 등의 데이터를 처리합니다.
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;


	UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
	TSubclassOf<UItemTooltipWidget> TooltipClass;

	UPROPERTY()
	UItemTooltipWidget* CachedTooltip;
};