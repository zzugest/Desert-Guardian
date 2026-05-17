#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuffIconWidget.generated.h"

// 전방 선언 
class UImage;
class UProgressBar;
class UTextBlock;
class UDataTable; 

UCLASS()
class THIRDGAME_API UBuffIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 외부(버프 목록 위젯)에서 이 버프의 정보를 세팅해 줄 함수
	UFUNCTION(BlueprintCallable, Category = "Buff")
	void InitBuff(UTexture2D* InIcon, float InMaxDuration, float InRemainingTime);

protected:
	// ==========================================
	// UI 바인딩
	// ==========================================
	UPROPERTY(meta = (BindWidget))
	UImage* BuffIconImage;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* CooldownBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;
	

private:
	// 내부적으로 기억할 데이터
	FName BuffID;
	float MaxDuration;
	float RemainingTime;
};