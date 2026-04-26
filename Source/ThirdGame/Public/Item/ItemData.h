// =========================================================================================
// ItemData.h
//
// [���� ���]
// ���� ���� �����ϴ� ��� �������� ������ ����(����, ����, Ư�� ��)�� ���� ��ٱ��� ����� ����ü�� �����ϴ� ����Դϴ�.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "ItemEffectBase.h"
#include "ItemData.generated.h" 

class UNiagaraSystem;

// �ý��� �󿡼� �������� ������ �з��ϱ� ���� ������ �������Դϴ�.
UENUM(BlueprintType)
enum class EItemType : uint8
{
	Weapon UMETA(DisplayName = "Weapon"),
	Potion UMETA(DisplayName = "Potion"),
};

// ==========================================================
// ������ ������ ����ü
// ==========================================================
// ��ȹ�ڰ� ������ ���̺��� �ۼ��� ���� ������ ������ ���� ��(Row) �����Դϴ�.
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// �ΰ��� UI � ��µ� �������� �̸��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FString ItemName = TEXT("None");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FString ItemKoreaName = TEXT("None");

	// �κ��丮�� ������ � ǥ�õ� �������� �ð��� ����(�ؽ�ó)�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	class UTexture2D* ItemIcon = nullptr;

	// �� �������� ��������, �Һ������� ���� �����ϴ� ī�װ����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	EItemType ItemType = EItemType::Weapon;

	// UI ���� � ǥ���� ������ �÷��̹� �ؽ�Ʈ �� �� �����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FString Description = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (MultiLine = "true"))
	FString EffectDescription = TEXT("");

	// ���� ���� ������ �������� �ϳ��� �κ��丮 ĭ�� ���ļ�(����) ������ �� �ִ��� �����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	bool bIsStackable = false;

	// ������ ���� ��� �ϳ��� ���Կ� �ִ�� �׾Ƶ� �� �ִ� ���� �����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 MaxStackSize = 1;

	// ���� ���� ���̰ų� �ٴڿ� ������ �ش� �������� ���� �����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 Quantity = 1;

	// �ν��Ͻ�ȭ�� �����۵��� ���������� ���� �� �����ϱ� ���� �߱޵Ǵ� ���� �ĺ� ��ȣ(GUID)�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FGuid ItemID;

	// ���� ���� �Ҹ� �������� ������� �� ĳ���� �ֺ��� ����� ��ƼŬ(���̾ư���) ����Ʈ�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	UNiagaraSystem* BuffEffect = nullptr;

	// ������ GUID ������ ���� �⺻ �������Դϴ�.
	FItemData() : ItemID(FGuid()) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TSubclassOf<class UItemEffectBase> ItemEffectClass = nullptr;

	// ���� �ŷ� �� �Ÿ��� ������ �� �������� ��ġ(���)�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 ItemPrice = 100;
};

// ==========================================================
// ��ٱ��� ������ ����ü
// ==========================================================
// ���� �ý��ۿ��� �ٷ��� ǰ���� �� ���� �����ϱ� ���� �ӽ÷� ������� ����� �����Դϴ�.
USTRUCT(BlueprintType)
struct FCartItem
{
	GENERATED_BODY()

public:
	// ���Ÿ� ����� ��� �������� ���� ���� �������Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cart")
	FItemData Data;

	// ��ٱ��Ͽ� ���� �ش� �������� ���� ��� �����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cart")
	int32 Quantity = 0;
};

