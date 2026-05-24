// =========================================================================================
// MoneyComponent.h
//
// [역할 요약]
// 캐릭터에 부착되어 실제 금액 데이터를 관리하는 전역 시스템(MoneySubsystem)과의 소통을 돕는 금전 편의 컴포넌트 헤더입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "MoneyComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THIRDGAME_API UMoneyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMoneyComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Gold();

	// 클라이언트 UX용: 현재 잔액으로 구매 가능한지 확인만 합니다. 실제 차감은 서버(PayGoldInternal)에서 처리합니다.
	UFUNCTION(BlueprintCallable, Category = "Money")
	bool TryBuyItem(int32 Cost);

	// 서버에서만 호출: 골드를 추가합니다.
	void AddGoldInternal(int32 Amount);

	// 서버에서만 호출: 골드 잔액을 확인하고 차감합니다. 성공 여부를 반환합니다.
	bool PayGoldInternal(int32 Amount);

	// 현재 소지금에 획득한 금액(Amount)을 추가시킵니다.
	UFUNCTION(BlueprintCallable, Category = "Money")
	void AddMoney(int32 Amount);

	// UI 등에 표시하기 위해 현재 소지 중인 총 금액을 조회하여 반환합니다.
	UFUNCTION(BlueprintPure, Category = "Money")
	int32 GetCurrentMoney() const;

	// 서버 권위 골드 데이터. 소유 클라이언트에만 복제됩니다.
	UPROPERTY(ReplicatedUsing=OnRep_Gold, VisibleAnywhere, BlueprintReadOnly, Category = "Money")
	int32 CurrentGold = 1000;
};
