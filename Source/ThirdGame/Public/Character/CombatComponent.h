
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h" 
#include "MyGameTypes.h"
#include "CombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatBuffUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDamageTaken);

USTRUCT(BlueprintType)
struct FActiveBuffInfo
{
    GENERATED_BODY()

    UPROPERTY()
    FName BuffID;

    UPROPERTY()
    float Amount = 0.0f;

    UPROPERTY()
    float MaxDuration = 0.0f;

    UPROPERTY()
    bool bIsItemBuff = false;
};

class AMyCharacter;
class AMySword;
class UAnimMontage;
class UTargetingComponent;
class USkeletalMeshComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THIRDGAME_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditDefaultsOnly, Category = "Data")
    UDataTable* AttackDataTable;

    FName CurrentAttackID;

    UPROPERTY()
    TSet<AActor*> AlreadyHitActors;

    void ResetMagicCombo();

    void OnMagicMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    bool bIsMagicComboWindowOpen = false;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Magic")
    UAnimMontage* MagicComboMontage;

    int32 MaxMagicCombo = 3;

    FTimerHandle MagicComboTimerHandle;

    FVector CachedMagicLocation;

    UPROPERTY()
    UTargetingComponent* CachedTargetingComp;

    UPROPERTY()
    USkeletalMeshComponent* CachedWeaponMesh;

    FAttackData* CachedAttackData = nullptr;

public:
    int32 CurrentMagicCombo = 0;

    UPROPERTY()
    AActor* CombatTargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float MaxHP = 100.0f;

    UPROPERTY(ReplicatedUsing=OnRep_Stats, EditAnywhere, BlueprintReadWrite, Category = "Status")
    float CurrentHP = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float MaxMP = 100.0f;

    UPROPERTY(ReplicatedUsing=OnRep_Stats, EditAnywhere, BlueprintReadWrite, Category = "Status")
    float CurrentMP = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float ManaRegenRate = 5.0f;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Status")
    float BaseAttackPower = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<AMySword> SwordClass;

    UPROPERTY(VisibleInstanceOnly, Category = "Combat")
    AMySword* CurrentWeapon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bIsWeaponEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    UAnimMontage* ComboActionMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    UAnimMontage* JumpAttackMontage;

    int32 CurrentCombo = 0;

    int32 MaxCombo = 3;

    bool bIsComboInputOn = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float MaxCooldown_Q = 10.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
    float CurrentCooldown_Q = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* SkillMontage;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void Attack();

    void SkillQ();

    void PlayComboAnim();

    void PlayComboMontageOnly(int32 ComboIndex);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ProcessComboCommand();

    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetWeaponCollision(bool bEnable);

    AMyCharacter* GetCharacterOwner();

    void UpdateStatToSubsystem();

    UFUNCTION(BlueprintCallable, Category = "Combat|Stats")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Combat|Stats")
    void RestoreMana(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Combat|Stats")
    void IncreaseAttackPower(float Amount);

    void PlayJumpAttackAnim();
    void PlayJumpLoopAnim();
    void PlayJumpLandingAnim();

    void ExecuteAttackHit(FName HitAttackID);

    void StartWeaponTrace(FName HitAttackID);
    void WeaponTraceTick();
    void EndWeaponTrace();

    void ReceiveDamage(float DamageAmount);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxSP;

    UPROPERTY(ReplicatedUsing=OnRep_Stats, EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentSP;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackTrackingSpeed = 8.0f;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_Stats();

    UFUNCTION(BlueprintCallable, Category = "Combat|Magic")
    void RightClickMagicAttack();

    void PlayMagicMontageOnly(int32 ComboIndex, FVector TargetLocation = FVector::ZeroVector);

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|VFX")
    class UNiagaraSystem* HitEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|VFX")
    class UParticleSystem* HitEffectCascade;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Magic|VFX")
    TArray<class UNiagaraSystem*> MagicExplosionVFXs;

    UFUNCTION(BlueprintCallable, Category = "Combat|Magic")
    void SpawnMagicVFX();

    UFUNCTION(BlueprintCallable, Category = "Combat|Magic")
    void ApplyMagicDamage();

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Buff")
    bool bIsAttackBuffActive = false;

    float ActiveAttackBuffAmount = 0.0f;

    FTimerHandle AttackBuffTimerHandle;

    void RemoveAttackBuff();

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCombatBuffUpdated OnCombatBuffUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDamageTaken OnDamageTaken;

private:
    float PrevHP = 0.f;

public:

    UPROPERTY()
    FName ActiveAttackBuffID;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Buff")
    bool bIsItemAttackBuffActive = false;

    float ActiveItemAttackBuffAmount = 0.0f;
    FTimerHandle ItemAttackBuffTimerHandle;
    void RemoveItemAttackBuff();

    float ItemBuffClientEndTime = 0.0f;

    UPROPERTY()
    FName ActiveItemAttackBuffID;

    void ApplyAttackBuff(float Amount, float Duration, FName BuffID, bool bFromItem = false);

    void LogBuffStatus();
    FTimerHandle BuffLogTimerHandle;

    UPROPERTY(ReplicatedUsing=OnRep_ActiveBuffs)
    TArray<FActiveBuffInfo> ActiveBuffs;

    UFUNCTION()
    void OnRep_ActiveBuffs();

    void SpawnHitEffectAt(FVector HitLocation, FRotator HitNormal);

    void ServerApplyHitDamage(AActor* HitEnemy, FVector HitLocation, FRotator HitNormal, FName AttackID);

    void ServerApplyMagicDamage(const TArray<AActor*>& HitEnemies, FName MagicAttackID);
};
