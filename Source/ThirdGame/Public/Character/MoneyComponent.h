// =========================================================================================
// MoneyComponent.h
//
// [역할 요약]
// 캐릭터에 부착되어 실제 금액 데이터를 관리하는 전역 시스템(MoneySubsystem)과의 소통을 돕는 금전 편의 컴포넌트 헤더입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MoneyComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THIRDGAME_API UMoneyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMoneyComponent();

	// 지정한 비용(Cost)만큼 소지금을 차감하여 지불 성공 여부를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Money")
	bool TryBuyItem(int32 Cost);

	// 현재 소지금에 획득한 금액(Amount)을 추가시킵니다.
	UFUNCTION(BlueprintCallable, Category = "Money")
	void AddMoney(int32 Amount);

	// UI 등에 표시하기 위해 현재 소지 중인 총 금액을 조회하여 반환합니다.
	UFUNCTION(BlueprintPure, Category = "Money")
	int32 GetCurrentMoney() const;
};