// =========================================================================================
// BossHPWidget.cpp
//
// [파일 역할]
// 보스 전투 중 화면에 표시되는 보스 전용 HP 바 위젯입니다.
// 피격 시 즉각 감소, 힐 시 부드러운 보간(FInterpTo) 으로 자연스러운 HP 변화를 표현합니다.
// 1페이즈↔2페이즈 전환 시 보스 이름 텍스트도 함께 갱신합니다.
// =========================================================================================

#include "BossHPWidget.h"
#include "Components/ProgressBar.h"

// 새로운 HP 비율을 계산하고 피격/회복 방향에 따라 즉시 감소 또는 보간 증가를 결정합니다.
void UBossHPWidget::UpdateHP(float CurrentHP, float MaxHP)
{
	if (MaxHP <= 0.0f) return;

	float NewTargetPercent = CurrentHP / MaxHP;

	// 위젯이 처음 열릴 때(CurrentPercent == 0, TargetPercent == 1) 즉시 현재 HP로 초기화합니다.
	if (CurrentPercent <= 0.0f && TargetPercent == 1.0f)
	{
		CurrentPercent = NewTargetPercent;
		TargetPercent = NewTargetPercent;
		if (HPBar) HPBar->SetPercent(CurrentPercent);
		return;
	}

	// HP가 감소하면 즉시 반영합니다.
	if (NewTargetPercent < TargetPercent)
	{
		TargetPercent = NewTargetPercent;
		CurrentPercent = NewTargetPercent;
		if (HPBar) HPBar->SetPercent(CurrentPercent);
	}
	// HP가 증가하면 NativeTick에서 FInterpTo로 부드럽게 채워집니다.
	else if (NewTargetPercent > TargetPercent)
	{
		TargetPercent = NewTargetPercent;
	}
}

// 매 프레임 CurrentPercent를 TargetPercent로 보간해 HP 바를 부드럽게 채웁니다.
void UBossHPWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (HPBar)
	{
		CurrentPercent = FMath::FInterpTo(CurrentPercent, TargetPercent, InDeltaTime, 1.5f);
		HPBar->SetPercent(CurrentPercent);
	}
}

// 페이즈 전환 시 보스 이름 텍스트를 새 페이즈 이름으로 갱신합니다.
void UBossHPWidget::UpdateBossName(const FString& NewName)
{
	if (BossNameText)
	{
		BossNameText->SetText(FText::FromString(NewName));
	}
}
