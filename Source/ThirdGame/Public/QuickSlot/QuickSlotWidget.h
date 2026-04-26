// =========================================================================================
// QuickSlotWidget.h
//
// [���� ���]
// �÷��̾� ȭ�� �ϴܿ� ��ġ�Ǵ� ���� ������ UI ������ ����Դϴ�.
// ������/����/����Ű ǥ��, ��ٿ� �ð�ȭ �� �巡�� �� ����� ���� ���� �� ������ ��ü ����� �����մϴ�.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h" 
#include "InventoryDragVisual.h"
#include "QuickSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UItemTooltipWidget;

// ������ �巡�� �� ��� �� ����� ������ ��� �����ϱ� ���� ���� �ӽ� ȭ��(Payload) Ŭ�����Դϴ�.
UCLASS()
class UQuickSlotDragPayload : public UObject
{
	GENERATED_BODY()
public:
	// �巡�װ� ���۵� ���� �������� �ε����� �����Ͽ� ��� ���������� ��ó�� �ĺ��ϱ� ���� �����Դϴ�.
	UPROPERTY()
	int32 SourceSlotIndex = -1;
};

UCLASS()
class THIRDGAME_API UQuickSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// �� ���� �ν��Ͻ��� ����� ������ ������ ��ȣ(�ε���)�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickSlot")
	int32 QuickSlotIndex = 0;

	// �ܺο��� ���޹��� ������ �����͸� �������� ������ �����ܰ� ���� ǥ�ø� �����մϴ�.
	UFUNCTION(BlueprintCallable)
	void SetItem(const FItemData& NewItem);

	// ���� �ϴܿ� ǥ�õ� ���� ����Ű(����) �ؽ�Ʈ�� �����մϴ�.
	UFUNCTION(BlueprintCallable)
	void SetKeyNumber(int32 Number);

	// ���Կ� ��ϵ� �������� ���� �ܿ� ������ ǥ���ϴ� �����Դϴ�.
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CountText;

	// �� ������ �������� ��ٿ� ��� �ð��� ����Ͽ� Ÿ�̸� UI�� �����մϴ�.
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// ��ϵ� �������� �ð��� ����(�ؽ���)�� ǥ���ϴ� �̹��� �����Դϴ�.
	UPROPERTY(meta = (BindWidget))
	class UImage* IconImage;

	// �ش� �����Կ� ���ε� Ű���� ����Ű ��ȣ�� �����ִ� �ؽ�Ʈ �����Դϴ�.
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KeyText;

	// ������ �巡���ϴ� ���� ���콺 Ŀ���� ����ٴ� �ܻ�(���־�) UI Ŭ�����Դϴ�.
	UPROPERTY(EditDefaultsOnly, Category = "QuickSlot")
	TSubclassOf<UInventoryDragVisual> DragVisualClass;

	// ���� ���� �� ����Ű �ؽ�Ʈ�� �ʱ�ȭ�ϰ� ����ý����� ������ ���� �̺�Ʈ�� �����մϴ�.
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// �ٸ� �����̳� �κ��丮���� �巡���� �� �������� �� ���� ���� �������� ���� ��ȯ ������ ó���մϴ�.
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// ����ý����� �ֽ� �����͸� �о�� ������, ����, �� ���� ���� ���� �ð������� ����ȭ�մϴ�.
	void UpdateSlotDisplay();

	// ����ý����� ������ ���� �˸��� �޾��� �� UI ȭ�� ������ �����ϴ� �ݹ� �Լ��Դϴ�.
	UFUNCTION()
	void OnQuickSlotSystemUpdated();

	// ������ ��� �� ���� ��� �ð�(��ٿ�) ���� ������ ��Ӱ� ������ ��� ���������Դϴ�.
	UPROPERTY(meta = (BindWidget))
	class UImage* CooldownOverlay;

	// ���� ��� �ð��� �Ϸ�Ǳ���� ���� �ð��� �� ������ ǥ���ϴ� �ؽ�Ʈ �����Դϴ�.
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownText;

	// ���� ���� ������ Ŭ�� ���� ���¸� �Ǻ��Ͽ� �巡�� ���� �߻� ���θ� �����մϴ�.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// ���� ���콺 �巡�� ������ �����Ǿ��� �� �ش� ���� ������ ���� ������ ȭ��(Payload) ��ü�� �����մϴ�.
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
	TSubclassOf<UItemTooltipWidget> TooltipClass;

	UPROPERTY()
	UItemTooltipWidget* CachedTooltip;
};