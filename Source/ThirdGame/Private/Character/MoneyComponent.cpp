// =========================================================================================
// MoneyComponent.cpp
//
// [파일 역할]
// 캐릭터에 부착되어 MoneySubsystem(전역 골드 저장소)과 연결하는 컴포넌트입니다.
// 아이템 구매 시 골드 차감 가능 여부 확인, 골드 획득, 현재 잔액 조회 기능을 외부에 제공합니다.
// =========================================================================================

#include "MoneyComponent.h"
#include "MoneySubsystem.h"

// 단순 저장 컴포넌트이므로 Tick은 비활성화합니다.
UMoneyComponent::UMoneyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// 현재 잔액에서 Cost만큼 차감할 수 있으면 MoneySubsystem에 차감을 요청하고 true를 반환합니다.
bool UMoneyComponent::TryBuyItem(int32 Cost)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return false;

	UMoneySubsystem* MoneySys = GI->GetSubsystem<UMoneySubsystem>();
	if (!MoneySys) return false;

	return MoneySys->PayGold(Cost);
}

// MoneySubsystem에 골드 추가를 요청합니다. (적 처치 보상 등에서 호출)
void UMoneyComponent::AddMoney(int32 Amount)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return;

	UMoneySubsystem* MoneySys = GI->GetSubsystem<UMoneySubsystem>();
	if (!MoneySys) return;

	MoneySys->AddGold(Amount);
}

// UI 표시 등을 위해 MoneySubsystem에서 현재 골드 잔액을 조회해 반환합니다.
int32 UMoneyComponent::GetCurrentMoney() const
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return 0;

	UMoneySubsystem* MoneySys = GI->GetSubsystem<UMoneySubsystem>();
	if (!MoneySys) return 0;

	return MoneySys->GetGold();
}
