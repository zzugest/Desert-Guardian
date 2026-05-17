#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "PurchaseSlotWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class THIRDGAME_API UPurchaseSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 장바구니 데이터 설정 (아이템 + 수량)
	void SetCartData(FCartItem InCartItem);

	// 빈 칸으로 만들기
	void SetEmpty();

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* IconImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CountText; // 갯수 표시 (x5)
};