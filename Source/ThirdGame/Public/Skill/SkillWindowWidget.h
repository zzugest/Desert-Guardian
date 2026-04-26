#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "SkillWindowWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillWindowClosed);

class UUniformGridPanel;


UCLASS()
class THIRDGAME_API USkillWindowWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 창이 열릴 때/닫힐 때 호출되는 함수
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 스킬카드를 균등 3열로 배치하는 그리드 컨테이너
	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* SkillContainer;

	// ��Ͽ� �߰��� "��ų ī��" ������ (WBP_SkillListEntry)
	// �����Ϳ��� ����� �� �ְ� TSubclassOf ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> SkillEntryClass;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Close;

	UFUNCTION()
	void OnCloseButtonClicked();


public:
	// ������ �ۿ��� ��ų ����� ���ΰ�ħ �϶�� ������ �� �ְ�
	void RefreshUI();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSkillWindowClosed OnWindowClosed;

};