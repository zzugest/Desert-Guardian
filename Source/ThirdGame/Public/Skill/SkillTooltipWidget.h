#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Skill/SkillData.h"
#include "SkillTooltipWidget.generated.h"

// 전방 선언 (헤더 의존성 최소화)
class UTextBlock;

UCLASS()
class THIRDGAME_API USkillTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 외부(스킬 슬롯/스킬 창)에서 스킬 데이터와 플레이어 기본 공격력을 넘겨 툴팁을 초기화하는 함수
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void InitTooltip(const FSkillData& SkillInfo, float BaseAttackPower);

protected:
	// 스킬 한국어 이름 텍스트 — Blueprint에서 TextBlock 이름을 SkillNameText로 추가하면 자동 연결
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SkillNameText;

	// 쿨타임 텍스트 (이름 옆에 표시) — BindWidgetOptional이므로 블루프린트에서 없어도 컴파일 에러 없음
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SkillCooldownText = nullptr;

	// 설명 텍스트 — DataTable의 Description에 {damage} 플레이스홀더를 쓰면 실제 피해량으로 자동 치환
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SkillDescText;
};
