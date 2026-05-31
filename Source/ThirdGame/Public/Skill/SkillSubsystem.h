
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SkillData.h" 
#include "SkillComponent.h"
#include "SkillSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillSlotUpdatedSignature);

UCLASS()
class THIRDGAME_API USkillSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USkillSubsystem();

	FSkillData* GetSkillData(FName SkillID);

	UFUNCTION(BlueprintCallable)
	void EquipSkill(int32 SlotIndex, FName SkillID);

	UFUNCTION(BlueprintCallable)
	FName GetSkillIDInSlot(int32 SlotIndex);

	UDataTable* GetSkillDataTable() const { return SkillDataTable; }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSkillSlotUpdatedSignature OnSkillSlotUpdated;

	void SaveTemporaryData(const TMap<FName, float>& InCooldowns, const TArray<FActiveBuff>& InBuffs);

	void LoadTemporaryData(TMap<FName, float>& OutCooldowns, TArray<FActiveBuff>& OutBuffs);

	void SaveAttackBuffData(bool bActive, FName BuffID, float Amount, float InRemainingTime);

	void LoadAttackBuffData(bool& bActive, FName& BuffID, float& Amount, float& OutRemainingTime);

protected:
	UPROPERTY()
	UDataTable* SkillDataTable;

	UPROPERTY()
	TMap<int32, FName> QuickSkillSlots;

	UPROPERTY()
	TMap<FName, float> BackupCooldowns;

	UPROPERTY()
	TArray<FActiveBuff> BackupBuffs;

	UPROPERTY()
	bool BackupAttackBuffActive = false;

	UPROPERTY()
	FName BackupAttackBuffID;

	UPROPERTY()
	float BackupAttackBuffAmount = 0.0f;

	UPROPERTY()
	float BackupAttackBuffRemainingTime = 0.0f;
};
