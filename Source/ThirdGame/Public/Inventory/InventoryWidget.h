// InventoryWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "GameWidgetBase.h"
#include "Components/Button.h"
#include "InventoryWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryClosed);

class UTileView;

UCLASS()
class THIRDGAME_API UInventoryWidget : public UGameWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void UpdateUI(const TArray<FItemData>& Content, int32 Capacity);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInventoryClosed OnWindowClosed;

protected:

	// ������ ó�� ������ ��
	virtual void NativeConstruct() override;

	// ������ ȭ�鿡�� ���ŵ� ��
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UTileView* InventoryTileView;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Close;

	UFUNCTION()
	void OnCloseButtonClicked();

	// UpdateUI 호출마다 NewObject를 반복 생성하지 않기 위해 미리 만들어 재사용합니다.
	UPROPERTY()
	TArray<class UInventoryItemObject*> CachedItemObjects;

};