
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemData.h" 
#include "QuickSlotSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuickSlotSubsystemUpdated);

UCLASS()
class THIRDGAME_API UQuickSlotSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot")
	TArray<FItemData> QuickSlotContent;

	UPROPERTY(BlueprintAssignable, Category = "QuickSlot")
	FOnQuickSlotSubsystemUpdated OnQuickSlotUpdated;

public:
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void RegisterQuickSlot(int32 SlotIndex, const FItemData& InItemData);

	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void UseQuickSlot(int32 SlotIndex);

	void ApplyCooldown(UClass* EffectClass, float CooldownTime);

	float GetRemainingCooldown(UClass* EffectClass);

	void SwapQuickSlots(int32 IndexA, int32 IndexB);

	UFUNCTION()
	void SyncQuickSlotsWithInventory();

private:
	UPROPERTY()
	TMap<UClass*, double> CooldownMap;
};
