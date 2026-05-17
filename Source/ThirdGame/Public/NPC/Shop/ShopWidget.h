#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h" 
#include "GameWidgetBase.h"
#include "ShopWidget.generated.h"


// 전방 선언
class UUniformGridPanel;
class UShopSlotWidget;
class UPurchaseSlotWidget;
class UHorizontalBox;
class UTextBlock;
class UButton;

UCLASS()
class THIRDGAME_API UShopWidget : public UGameWidgetBase
{
	GENERATED_BODY()

public:
	// 상점 초기화
	UFUNCTION(BlueprintCallable)
	void InitShop(const TArray<FItemData>& SaleItems);

	// 장바구니에 아이템 담기 요청
	void AddToCart(FItemData ItemToAdd);


	UFUNCTION(BlueprintCallable, Category = "Shop UI")
	void OnClickClose();

	UShopWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

	// 위젯이 제거될 때 호출되는 함수
	virtual void NativeDestruct() override;

	// [UI 바인딩]
	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* ItemGrid;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* CartGrid;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalPriceText;

	UPROPERTY(meta = (BindWidget))
	UButton* FinalBuyButton;

	// [설정]
	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UShopSlotWidget> ShopSlotClass;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UPurchaseSlotWidget> PurchaseSlotClass;

	// [함수]
	void UpdateCartUI();
	void UpdateTotalPrice();

	UFUNCTION()
	void OnClickFinalBuy();


	UPROPERTY(meta = (BindWidget))
	class UButton* CloseButton;

private:
	// 장바구니 데이터
	TArray<FCartItem> CartItems;
};