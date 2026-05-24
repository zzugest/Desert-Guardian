// =========================================================================================
// MoneyComponent.cpp
//
// [파일 역할]
// 캐릭터에 부착되어 MoneySubsystem(전역 골드 저장소)과 연결하는 컴포넌트입니다.
// 아이템 구매 시 골드 차감 가능 여부 확인, 골드 획득, 현재 잔액 조회 기능을 외부에 제공합니다.
// =========================================================================================

#include "MoneyComponent.h"
#include "MoneySubsystem.h"
#include "Net/UnrealNetwork.h"

// 단순 저장 컴포넌트이므로 Tick은 비활성화합니다.
UMoneyComponent::UMoneyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
}

void UMoneyComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[LV_TRAVEL] MoneyComp BeginPlay | Auth:%s | Gold:%d"),
		(GetOwner() && GetOwner()->HasAuthority()) ? TEXT("SERVER") : TEXT("CLIENT"),
		CurrentGold);
}

// 복제할 변수를 등록합니다. COND_OwnerOnly로 소유 클라이언트에만 전송해 대역폭을 절약합니다.
void UMoneyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UMoneyComponent, CurrentGold, COND_OwnerOnly);
}

// Content 복제 수신 시 호출 — MoneySubsystem에 동기화하고 UI 갱신 델리게이트를 브로드캐스트합니다.
void UMoneyComponent::OnRep_Gold()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return;

	UMoneySubsystem* MoneySys = GI->GetSubsystem<UMoneySubsystem>();
	if (!MoneySys) return;

	MoneySys->SetGold(CurrentGold);

	UE_LOG(LogTemp, Log, TEXT("[MONEY_SYNC][CLIENT][%s] OnRep received -> MoneySubsystem synced (gold: %d)"),
		*GetOwner()->GetName(), CurrentGold);
}

// 클라이언트 UX용 사전 검사입니다. 실제 차감은 서버(PayGoldInternal)에서 처리합니다.
bool UMoneyComponent::TryBuyItem(int32 Cost)
{
	return CurrentGold >= Cost;
}

// 서버에서만 호출합니다. 골드를 추가하고 복제를 트리거합니다.
void UMoneyComponent::AddGoldInternal(int32 Amount)
{
	if (Amount <= 0) return;

	int32 Before = CurrentGold;
	CurrentGold += Amount;

	UE_LOG(LogTemp, Log, TEXT("[MONEY_SYNC][SERVER][%s] AddGold: +%d | before: %d | after: %d"),
		*GetOwner()->GetName(), Amount, Before, CurrentGold);
}

// 서버에서만 호출합니다. 잔액 검증 후 차감합니다. 성공 여부를 반환합니다.
bool UMoneyComponent::PayGoldInternal(int32 Amount)
{
	if (Amount <= 0) return false;

	if (CurrentGold < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MONEY_SYNC][SERVER][%s] PayGold FAILED: cost %d | balance: %d (not enough)"),
			*GetOwner()->GetName(), Amount, CurrentGold);
		return false;
	}

	int32 Before = CurrentGold;
	CurrentGold -= Amount;

	UE_LOG(LogTemp, Log, TEXT("[MONEY_SYNC][SERVER][%s] PayGold: -%d | before: %d | after: %d"),
		*GetOwner()->GetName(), Amount, Before, CurrentGold);

	return true;
}

// MoneySubsystem에 골드 추가를 요청합니다. (적 처치 보상 등에서 호출)
void UMoneyComponent::AddMoney(int32 Amount)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		AddGoldInternal(Amount);
	}
}

// UI 표시 등을 위해 현재 골드 잔액을 반환합니다.
int32 UMoneyComponent::GetCurrentMoney() const
{
	return CurrentGold;
}
