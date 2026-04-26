#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillSlotWidget.generated.h"

// 전방 선언
class UImage;
class UProgressBar;
class UTextBlock;
class USkillComponent;
class UCombatComponent;
class USkillTooltipWidget;

UCLASS()
class THIRDGAME_API USkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// �� ������ �� �� �������� (0=Q, 1=E, 2=R)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Slot")
	int32 SlotIndex = 0;

	// �����Ϳ��� "Q", "E" �Է��� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Slot")
	FText KeyName;

	// UI ���� �Լ� (������ ���� ��)
	UFUNCTION(BlueprintCallable)
	void UpdateSlotInfo();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop")
	TSubclassOf<class UUserWidget> DragVisualClass;

	// 마우스 오버 시 표시할 스킬 툴팁 위젯 클래스 (블루프린트에서 WBP_SkillTooltip 할당)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
	TSubclassOf<USkillTooltipWidget> SkillTooltipClass;



protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativePreConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	// 마우스가 슬롯 위에 올라올 때 최신 공격력 기준으로 툴팁을 갱신합니다.
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;



	// [UI ���ε�]
	UPROPERTY(meta = (BindWidget))
	UImage* SkillIcon;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* CooldownBar; // ��Ÿ�� ǥ�ÿ� (��ο� ��)

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* CooldownOverlay; // ��Ÿ�� �߿� ���ĴϿ� �̹���

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* CooldownText; // ��Ÿ�� ���� �ʰ� ǥ�� �ؽ�Ʈ

	UPROPERTY(meta = (BindWidget))
	UTextBlock* KeyText; // "Q", "E" ǥ��


private:
	// 쿨타임 계산을 위해 매 틱 캐릭터의 컴포넌트를 알고 있어야 함
	UPROPERTY()
	USkillComponent* OwnerSkillComp;

	// 툴팁 피해량 계산을 위해 CombatComponent 캐싱
	UPROPERTY()
	UCombatComponent* OwnerCombatComp;

	// 현재 이 슬롯에 등록된 스킬 ID
	FName CurrentSkillID;

	// 마우스 오버 시 툴팁 위젯을 생성·초기화하여 SetToolTipWidget으로 등록합니다.
	void UpdateTooltip();
};