// =========================================================================================
// PlayerHUDWidget.cpp
//
// [파일 역할]
// 플레이어 화면 상단에 표시되는 메인 HUD UI 클래스입니다.
// HP·MP·SP 프로그레스 바를 실시간으로 갱신하고,
// MoneySubsystem의 골드 변경 델리게이트를 구독해 골드 텍스트를 자동으로 업데이트합니다.
// =========================================================================================

#include "PlayerHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MoneySubsystem.h"

// 현재 HP·MP·SP를 각 바의 비율(0~1)로 변환해 프로그레스 바에 반영합니다.
void UPlayerHUDWidget::UpdateState(float CurrentHP, float MaxHP, float CurrentMP, float MaxMP, float CurrentSP, float MaxSP)
{
	// 최대치가 0이면 0 나누기 오류가 발생하므로 0보다 클 때만 갱신합니다.
	if (HPBar && MaxHP > 0.0f)
	{
		HPBar->SetPercent(CurrentHP / MaxHP);
	}

	if (ManaBar && MaxMP > 0.0f)
	{
		ManaBar->SetPercent(CurrentMP / MaxMP);
	}

	if (SPBar && MaxSP > 0.0f)
	{
		SPBar->SetPercent(CurrentSP / MaxSP);
	}
}

// 위젯 생성 시 MoneySubsystem의 골드 변경 이벤트를 구독하고, 초기 골드 수치를 표시합니다.
void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UMoneySubsystem* MoneySys = GI->GetSubsystem<UMoneySubsystem>();
	if (!MoneySys) return;

	MoneySys->OnGoldChanged.AddDynamic(this, &UPlayerHUDWidget::OnPlayerMoneyChanged);

	// 위젯이 처음 열릴 때 현재 잔액을 즉시 표시합니다.
	OnPlayerMoneyChanged(MoneySys->GetGold());
}

// 위젯 파괴 시 MoneySubsystem 델리게이트를 해제해 크래시를 방지합니다.
void UPlayerHUDWidget::NativeDestruct()
{
	Super::NativeDestruct();

	// HUD가 파괴될 때(레벨 전환 등) MoneySubsystem 델리게이트를 해제합니다.
	// 해제하지 않으면 Subsystem이 파괴된 HUD의 함수를 호출해 크래시가 발생할 수 있습니다.
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UMoneySubsystem* MoneySys = GI->GetSubsystem<UMoneySubsystem>();
	if (!MoneySys) return;

	MoneySys->OnGoldChanged.RemoveDynamic(this, &UPlayerHUDWidget::OnPlayerMoneyChanged);
}

// 골드 변경 이벤트를 수신해 골드 텍스트 UI를 최신 수치로 갱신합니다.
void UPlayerHUDWidget::OnPlayerMoneyChanged(int32 NewGold)
{
	if (!GoldText) return;

	GoldText->SetText(FText::AsNumber(NewGold));
}
