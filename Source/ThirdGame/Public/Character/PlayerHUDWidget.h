// =========================================================================================
// PlayerHUDWidget.h
//
// [���� ���]
// �÷��̾� ȭ�鿡 ��� ��ġ�Ǵ� �ٽ� UI ���� ����Դϴ�. ����(ü��/����/���׹̳�) ���α׷��� �� ���� �� ������(���) �ؽ�Ʈ ������Ʈ�� ����մϴ�.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.h"
#include "PlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class THIRDGAME_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// ĳ������ ���� ���¸� �޾ƿ� ���α׷��� ���� ������ ���� �����(Ratio)�� �����մϴ�.
	void UpdateState(float CurrentHP, float MaxHP, float CurrentMP, float MaxMP, float CurrentSP, float MaxSP);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ü�� ���¸� ǥ���ϴ� ������ �� �����Դϴ�.
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPBar;

	// ���� ���¸� ǥ���ϴ� ������ �� �����Դϴ�.
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ManaBar;

	// ���¹̳� ���¸� ǥ���ϴ� ������ �� �����Դϴ�.
	UPROPERTY(meta = (BindWidget))
	UProgressBar* SPBar;

	// ���� ������ ��ȭ(���) ���� ��Ÿ���� �ؽ�Ʈ �����Դϴ�.
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GoldText;

	// ��ȭ ����ý������κ��� �ܾ� ���� �˸��� ���� �� �ؽ�Ʈ UI�� ���ΰ�ħ�մϴ�.
	UFUNCTION()
	void OnPlayerMoneyChanged(int32 NewGold);
};