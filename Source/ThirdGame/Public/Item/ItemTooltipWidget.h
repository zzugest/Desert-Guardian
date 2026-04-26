#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"
#include "ItemTooltipWidget.generated.h"

// ���� ���� (������ �ӵ� ����ȭ)
class UTextBlock;

UCLASS()
class THIRDGAME_API UItemTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//  �ܺ�(�κ��丮 ���� ��)���� ������ �����͸� ��°�� �Ѱ��� �� ����� �Լ�
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void InitTooltip(const FItemData& ItemInfo);

protected:
	//  UI �����̳�(��������Ʈ)���� ���� �ؽ�Ʈ ���ϵ�� 1:1�� ���� �������Դϴ�.
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText;      // �ѱ��� �̸��� �� ��

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemDescText;      // ��� ������ �� ��

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemEffectText;    // ȿ�� ������ �� ��

	// 쿨타임 표시 텍스트 — 블루프린트에서 TextBlock 이름을 ItemCooldownText로 추가하면 자동 연결
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ItemCooldownText = nullptr;
};
