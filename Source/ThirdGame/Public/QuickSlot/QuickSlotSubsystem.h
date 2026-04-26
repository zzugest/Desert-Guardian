// =========================================================================================
// QuickSlotSubsystem.h
//
// [���� ���]
// ���� �������� �÷��̾��� ������ �����͸� ���� �����ϰ�, �κ��丮 �ý��۰� ������ ������ ��� ��ٿ� �� ���� ����ȭ�� �����ϴ� ����ý��� ����Դϴ�.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemData.h" 
#include "QuickSlotSubsystem.generated.h"

// ������ ����̳� ���� �����Ͱ� ���ŵǾ��� �� ȭ�� UI�� ���ΰ�ħ�ϱ� ���� ȣ��Ǵ� ��������Ʈ�Դϴ�.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuickSlotSubsystemUpdated);

UCLASS()
class THIRDGAME_API UQuickSlotSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ����ý��� ���� �� ������ �迭 ����� �Ҵ��ϰ� �⺻ ĭ(����)�� ä�� �ʱ�ȭ�մϴ�.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ���� �÷��̾ �����Կ� ����Ͽ� ���� ���� ���� �����۵��� �迭 ����Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot")
	TArray<FItemData> QuickSlotContent;

	// ������ ���� ���� ������ �߻����� �� UI�� ȭ�� ����ȭ�� �����ϱ� ���� �뺸�� �̺�Ʈ�Դϴ�.
	UPROPERTY(BlueprintAssignable, Category = "QuickSlot")
	FOnQuickSlotSubsystemUpdated OnQuickSlotUpdated;

public:
	// ��� ������ �ε����� ���ο� ������ �����͸� ����� ����մϴ�.
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void RegisterQuickSlot(int32 SlotIndex, const FItemData& InItemData);

	// �ش� �ε����� ��ϵ� �������� ���� �κ��丮���� ã�� ������ �����ϰ� ȿ���� ����(���)�մϴ�.
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void UseQuickSlot(int32 SlotIndex);

	// ��� ������(ȿ��)�� ���� ������ �������� ��ٿ� ���� ���� ���� �ð��� ����� ���� ���忡 ����մϴ�.
	void ApplyCooldown(UClass* EffectClass, float CooldownTime);

	// ���� �ð��� ��ٿ� ��� ������ �����Ͽ�, ��� �������� ���� ���ɱ��� ���� �ð��� ��ȯ�մϴ�.
	float GetRemainingCooldown(UClass* EffectClass);

	// �巡�� �� ��� ���� ���� ���� �� ������ ���� ������ ������ ������ ��ġ�� ���� �¹ٲߴϴ�.
	void SwapQuickSlots(int32 IndexA, int32 IndexB);

	// ������ �� ���� �κ��丮 ������ ������ ������ ��, ��ϵ� �������� ǥ�� ������ �����ϰ� ����ȭ��ŵ�ϴ�.
	UFUNCTION()
	void SyncQuickSlotsWithInventory();

private:
	// ���� ��� �� ������ ���� ���� ������(Ŭ����)�� ��ٿ� ���� �ð��� �����ϴ� �ؽ� �� ����Դϴ�.
	UPROPERTY()
	TMap<UClass*, double> CooldownMap;
};