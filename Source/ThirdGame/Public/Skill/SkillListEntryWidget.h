#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h" 
#include "SkillListEntryWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
class USkillDragVisual;
class UCombatComponent;

UCLASS()
class THIRDGAME_API USkillListEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// �� �Լ��� ȣ���ϸ� �����ܰ� �̸��� ���ŵ�
	UFUNCTION(BlueprintCallable)
	void UpdateUI(FName SkillID);

protected:
	virtual void NativeConstruct() override;

	
	UPROPERTY(meta = (BindWidget))
	UImage* SkillIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SkillName;

	// ���� �� ī�尡 ��� �ִ� ��ų ID
	UPROPERTY(BlueprintReadOnly)
	FName CurrentSkillID;


	// 마우스 클릭 처리
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 드래그 처리
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	// 마우스가 올라올 때 툴팁을 최신 공격력 기준으로 갱신합니다.
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 드래그할 때 쓰는 마우스 커서 위에 표시되는 것 (블루프린트에서 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drag Drop")
	TSubclassOf<USkillDragVisual> DragVisualClass;

	// 마우스 오버 시 표시할 스킬 툴팁 위젯 클래스 (블루프린트에서 WBP_SkillTooltip 할당)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
	TSubclassOf<class USkillTooltipWidget> SkillTooltipClass;

private:
	// NativeConstruct에서 한 번만 캐싱 — hover마다 FindComponentByClass 호출 방지
	UPROPERTY()
	UCombatComponent* CachedCombatComp = nullptr;
};