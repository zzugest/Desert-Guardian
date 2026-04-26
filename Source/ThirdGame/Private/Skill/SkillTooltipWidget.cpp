#include "SkillTooltipWidget.h"
#include "Components/TextBlock.h"

void USkillTooltipWidget::InitTooltip(const FSkillData& SkillInfo, float BaseAttackPower)
{
	// 1. 한국어 이름 세팅
	if (SkillNameText)
	{
		SkillNameText->SetText(SkillInfo.SkillKoreaName);
	}

	// 2. 쿨타임 텍스트 세팅
	if (SkillCooldownText)
	{
		if (SkillInfo.Cooldown > 0.0f)
		{
			FString CooldownStr = FString::Printf(TEXT("(쿨타임 : %.0f초)"), SkillInfo.Cooldown);
			SkillCooldownText->SetText(FText::FromString(CooldownStr));
			SkillCooldownText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			SkillCooldownText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 3. 설명 텍스트 세팅
	// DataTable의 Description에 {damage} 플레이스홀더가 있으면 실제 피해량으로 치환합니다.
	// 예시 DataTable 입력: "얼음 화살을 대상에게 쏘아 {damage} 피해를 줍니다."
	//           실제 표시: "얼음 화살을 대상에게 쏘아 250 피해를 줍니다."
	if (SkillDescText)
	{
		int32 DamageValue     = FMath::RoundToInt(BaseAttackPower * SkillInfo.DamageMultiplier);
		int32 DurationValue   = FMath::RoundToInt(SkillInfo.Duration);
		int32 BuffAmountValue = FMath::RoundToInt(SkillInfo.BuffAmount);

		FFormatNamedArguments Args;
		Args.Add(TEXT("damage"),      FText::AsNumber(DamageValue));
		Args.Add(TEXT("duration"),    FText::AsNumber(DurationValue));
		Args.Add(TEXT("buff_amount"), FText::AsNumber(BuffAmountValue));

		FText FormattedDesc = FText::Format(SkillInfo.Description, Args);
		SkillDescText->SetText(FormattedDesc);
	}
}
