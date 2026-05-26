#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuffListWidget.generated.h"

// ���� ����
class UHorizontalBox;
class UBuffIconWidget;
class USkillComponent;
class UCombatComponent;

UCLASS()
class THIRDGAME_API UBuffListWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI ���ε� ��������Ʈ���� ���� ���� �ڽ�
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* BuffBox;

	// ������ ���� ���� ������ ������ Ŭ����
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UBuffIconWidget> BuffIconClass;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	class UDataTable* SkillDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	class UDataTable* ItemDataTable;

public:
	// ��۱�(Delegate)���� ������ ���� ����� �Լ�
	UFUNCTION()
	void UpdateBuffList();

	// 1초마다 버프 UI 표시 시간을 로그로 출력합니다.
	UFUNCTION()
	void LogBuffUIStatus();

private:
	// 매번 찾지 않도록 NativeConstruct에서 캐싱
	UPROPERTY()
	USkillComponent* CachedSkillComp;

	UPROPERTY()
	UCombatComponent* CachedCombatComp;

	FTimerHandle BuffUILogTimerHandle;
};