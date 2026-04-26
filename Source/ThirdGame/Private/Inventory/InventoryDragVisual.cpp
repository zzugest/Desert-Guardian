// =========================================================================================
// InventoryDragVisual.cpp
//
// [파일 역할]
// 인벤토리 슬롯을 드래그할 때 마우스 커서를 따라다니는 아이템 아이콘 위젯입니다.
// NativeOnDragDetected에서 생성되어 DragDropOperation의 시각 요소로 사용됩니다.
// =========================================================================================

#include "Inventory/InventoryDragVisual.h"
#include "Components/Image.h"

// 드래그 중 커서 옆에 표시할 아이템 아이콘 텍스처를 설정합니다.
void UInventoryDragVisual::SetDragIcon(UTexture2D* IconTexture)
{
	if (DragIconImage && IconTexture)
	{
		DragIconImage->SetBrushFromTexture(IconTexture);
		// 드래그 비주얼이 드롭 이벤트를 가로채지 않도록 HitTestInvisible로 설정합니다.
		DragIconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}
