
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "ItemEffectBase.h"
#include "ItemData.generated.h" 

class UNiagaraSystem;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Weapon UMETA(DisplayName = "Weapon"),
	Potion UMETA(DisplayName = "Potion"),
};

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FString ItemName = TEXT("None");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FString ItemKoreaName = TEXT("None");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	class UTexture2D* ItemIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	EItemType ItemType = EItemType::Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FString Description = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (MultiLine = "true"))
	FString EffectDescription = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	bool bIsStackable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 MaxStackSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FGuid ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	UNiagaraSystem* BuffEffect = nullptr;

	FItemData() : ItemID(FGuid()) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TSubclassOf<class UItemEffectBase> ItemEffectClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 ItemPrice = 100;
};

USTRUCT(BlueprintType)
struct FCartItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cart")
	FItemData Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cart")
	int32 Quantity = 0;
};

