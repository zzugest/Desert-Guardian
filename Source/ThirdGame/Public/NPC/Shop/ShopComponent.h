#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemData.h"
#include "ShopComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THIRDGAME_API UShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShopComponent();

	// 이 NPC가 판매할 아이템 목록 (에디터에서 직접 추가 가능하게 설정)	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TArray<FItemData> SaleItems;

	
};