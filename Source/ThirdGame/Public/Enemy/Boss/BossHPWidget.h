#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "BossHPWidget.generated.h"

UCLASS()
class THIRDGAME_API UBossHPWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 체력바를 업데이트할 함수
	void UpdateHP(float CurrentHP, float MaxHP);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BossNameText;

	void UpdateBossName(const FString& NewName);

protected:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 블루프린트 위젯(UMG)에서 이름이 "HPBar"인 프로그래스바를 자동으로 연결해줍니다.
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HPBar;


private:
	// 위젯이 스스로 기억할 체력 비율 변수들
	float TargetPercent = 1.0f;  // 보스가 알려준 진짜 체력 (목표치)
	float CurrentPercent = 0.0f; // 현재 화면에 그려지고 있는 체력 (스르륵 움직일 녀석)
};