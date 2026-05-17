// EnemyHPBarWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "EnemyHPBarWidget.generated.h" 

UCLASS()
class THIRDGAME_API UEnemyHPBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 체력바를 갱신하는 함수
	void UpdateHPWidget(float CurrentHP, float MaxHP);

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TXT_EnemyName;

protected:
	
	
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HPProgressBar;
};