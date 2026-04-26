#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MyGameTypes.generated.h"

// 공격 데미지 데이터를 담을 구조체
USTRUCT(BlueprintType)
struct FAttackData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float DamageMultiplier;
};