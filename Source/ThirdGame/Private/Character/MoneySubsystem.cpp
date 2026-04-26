// =========================================================================================
// MoneySubsystem.cpp
//
// [파일 역할]
// 게임 전역에서 플레이어의 골드(재화)를 관리하는 GameInstance 서브시스템입니다.
// 골드 추가·차감·조회 기능을 제공하며, 변경 시 OnGoldChanged 델리게이트로 UI에 알립니다.
// =========================================================================================

#include "MoneySubsystem.h"

// 지정한 양의 골드를 현재 보유량에 더하고, 변경 사실을 델리게이트로 브로드캐스트합니다.
void UMoneySubsystem::AddGold(int32 Amount)
{
	if (Amount <= 0) return;

	CurrentGold += Amount;

	if (OnGoldChanged.IsBound())
	{
		OnGoldChanged.Broadcast(CurrentGold);
	}
}

// 골드가 충분하면 지정한 금액을 차감하고 true를 반환합니다. 잔액 부족 시 false를 반환합니다.
bool UMoneySubsystem::PayGold(int32 Amount)
{
	if (Amount <= 0) return false;

	if (CurrentGold < Amount) return false;

	CurrentGold -= Amount;

	if (OnGoldChanged.IsBound())
	{
		OnGoldChanged.Broadcast(CurrentGold);
	}

	return true;
}

// 현재 보유 골드를 반환합니다.
int32 UMoneySubsystem::GetGold() const
{
	return CurrentGold;
}
