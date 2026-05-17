#include "Skill/BuffIconWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Skill/SkillData.h" // 만들어두신 스킬 구조체 헤더

// BuffIconWidget.cpp
// Purpose:
//   - 개별 버프 아이콘 위젯: 아이콘, 남은 시간 표시 및 프로그레스바 업데이트 담당.
// Key behaviors:
//   - InitBuff: 데이터 초기화 및 아이콘 설정(데이터테이블 참조).
//   - NativeTick: 남은 시간 감소, 프로그레스/텍스트 갱신.
// Safety notes:
//   - SkillDataTable, BuffIconImage, CooldownBar, TimeText 포인터 null 체크 필요.

void UBuffIconWidget::InitBuff(UTexture2D* InIcon, float InMaxDuration, float InRemainingTime)
{
	MaxDuration = InMaxDuration;
	RemainingTime = InRemainingTime;

	if (BuffIconImage && InIcon)
	{
		BuffIconImage->SetBrushFromTexture(InIcon);
	}
}

void UBuffIconWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 1) 남은 시간 갱신
	if (RemainingTime > 0.0f)
	{
		RemainingTime -= InDeltaTime;

		// 2) 프로그레스 바 갱신 (남은시간 / 최대시간)
		if (MaxDuration > 0.0f)
		{
			float Percent = RemainingTime / MaxDuration;
			CooldownBar->SetPercent(Percent);
		}

		// 3) 남은 시간 텍스트 갱신 (천장 올림하여 정수로 표기)
		int32 SecondsLeft = FMath::CeilToInt(RemainingTime);
		TimeText->SetText(FText::AsNumber(SecondsLeft));
	}
}