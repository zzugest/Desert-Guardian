
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuffListUpdatedSignature);

USTRUCT(BlueprintType)
struct FActiveBuff
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName BuffID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RemainingTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MaxDuration;
};

class USkillSubsystem;
class UCombatComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THIRDGAME_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void TryCastSkill(int32 SlotIndex);

	UFUNCTION(BlueprintCallable)
	float GetRemainingCooldown(FName SkillID);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetMaxCooldown(FName SkillID) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<int32, FName> QuickSlots;

	UFUNCTION(BlueprintCallable)
	void RegistSkillToSlot(int32 SlotIndex, FName SkillID);

	UFUNCTION(BlueprintCallable)
	FName GetSkillIDAtSlot(int32 SlotIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buff")
	TArray<FActiveBuff> ActiveBuffs;

	UPROPERTY(BlueprintAssignable, Category = "Buff")
	FOnBuffListUpdatedSignature OnBuffListUpdated;

	UFUNCTION(BlueprintCallable, Category = "Buff")
	void AddBuff(FName NewBuffID, float Duration);

	void SpawnProjectile(FName SkillID, FName SocketName);

private:
	UFUNCTION()
	void OnSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	TMap<FName, float> ActiveCooldowns;

	UPROPERTY()
	UCombatComponent* CombatComp;
};
