#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryDragVisual.generated.h"

UCLASS()
class THIRDGAME_API UInventoryDragVisual : public UUserWidget
{
	GENERATED_BODY()

public:
	// 외부에서 아이콘 텍스처를 넣어줄 함수
	void SetDragIcon(UTexture2D* IconTexture);

protected:
	// 위젯 블루프린트의 이미지와 연결
	UPROPERTY(meta = (BindWidget))
	class UImage* DragIconImage;
};