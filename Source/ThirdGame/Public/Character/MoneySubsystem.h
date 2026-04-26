// =========================================================================================
// MoneySubsystem.h
//
// [역할 요약]
// 게임 전체에서 유일무이한 인스턴스로 존재하며 플레이어의 실제 잔액(골드)을 훼손 없이 보관하고 조회할 수 있는 전역 중앙은행입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MoneySubsystem.generated.h"

// UI나 기타 정보 창에서 잔금이 바뀌었을 때 화면을 실시간 갱신할 수 있도록 해주는 방송(이벤트) 객체입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewGold);

UCLASS()
class THIRDGAME_API UMoneySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 재화 변동 발생 시 블루프린트로 즉각 파급을 알리는 알람 시스템 변수입니다.
	UPROPERTY(BlueprintAssignable, Category = "Money")
	FOnGoldChanged OnGoldChanged;

	// 전리품이나 퀘스트 보상으로 획득한 골드를 안전하게 누적시킵니다.
	UFUNCTION(BlueprintCallable, Category = "Money")
	void AddGold(int32 Amount);

	// 상점 결제 시 잔액을 확인하고 부족하지 않다면 보유 금액을 차감해 성공 여부를 돌려줍니다.
	UFUNCTION(BlueprintCallable, Category = "Money")
	bool PayGold(int32 Amount);

	// 현재 총 얼마나 많은 골드를 소유하고 있는지 조회합니다.
	UFUNCTION(BlueprintPure, Category = "Money")
	int32 GetGold() const;

private:
	// 외부 해킹이나 잘못된 참조로 값이 조작되는 것을 막기 위한 보호된 실제 잔고 데이터입니다.
	UPROPERTY()
	int32 CurrentGold = 1000;
};