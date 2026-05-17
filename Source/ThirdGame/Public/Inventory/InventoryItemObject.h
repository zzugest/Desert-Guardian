#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemData.h"
#include "InventoryItemObject.generated.h"

UCLASS(Blueprintable)
class THIRDGAME_API UInventoryItemObject : public UObject
{
	GENERATED_BODY()

public:
	// 아이템 데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FItemData ItemData;

	// 나는 인벤토리의 몇 번째 칸인가?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 Index = -1;
};