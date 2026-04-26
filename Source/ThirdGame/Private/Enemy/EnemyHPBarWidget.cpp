// =========================================================================================
// EnemyHPBarWidget.cpp
//
// [파일 역할]
// 적 캐릭터의 머리 위에 표시되는 HP 바 위젯입니다.
// Enemy::TakeDamage / Revive에서 호출되어 현재 HP 비율을 프로그레스 바에 반영합니다.
// =========================================================================================

#include "EnemyHPBarWidget.h"
#include "Components/ProgressBar.h"

// 현재 HP와 최대 HP를 받아 프로그레스 바 비율(0~1)을 갱신합니다.
void UEnemyHPBarWidget::UpdateHPWidget(float CurrentHP, float MaxHP)
{
	if (HPProgressBar && MaxHP > 0.0f)
	{
		float Percent = CurrentHP / MaxHP;
		HPProgressBar->SetPercent(Percent);
	}
}
