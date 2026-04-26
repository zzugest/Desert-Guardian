// =========================================================================================
// CombatComponent.h
//
// [���� ���]
// ĳ����(�÷��̾�)�� �����Ǿ� ����(HP, MP, SP) ����, ���� �ִϸ��̼�, �޺� �� ���� Ÿ�� ���� �� ������ ���õ� �������� �������̽��� ���� ������ �����ϴ� ������Ʈ ����Դϴ�.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h" 
#include "MyGameTypes.h"
#include "CombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatBuffUpdated);

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

    // ���� ID�� ��� �� ���� ������ ��� ���������̺�
    UPROPERTY(EditDefaultsOnly, Category = "Data")
    UDataTable* AttackDataTable;

    // ���� �ֵθ��� ������ ���� ���� �ĺ���
    FName CurrentAttackID;

    // �� ���� ���� ����(Ʈ���̽�)���� �ٴ� ��Ʈ�� �����ϱ� ���� �ǰ� ��� ���
    UPROPERTY()
    TSet<AActor*> AlreadyHitActors;


    // 2�� Ÿ�̸Ӱ� ����Ǿ��� �� ȣ��� �޺� �ʱ�ȭ �Լ�
    void ResetMagicCombo();

    // ���� ��Ÿ�ְ� ������ �� ó���� ���� ��������Ʈ �Լ�
    void OnMagicMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // ���� ���� �޺� �Է��� ��ٸ��� ������ üũ
    bool bIsMagicComboWindowOpen = false;

    // �����Ϳ��� ��� ���� ���� ��Ÿ�ָ� ���� ����
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Magic")
    UAnimMontage* MagicComboMontage;

    // ���� ���� �޺� �ܰ�
    int32 CurrentMagicCombo = 0;

    // �ִ� ���� �޺� �� (��: 3)
    int32 MaxMagicCombo = 3;

    // �𸮾� Ÿ�̸Ӹ� �����ϱ� ���� �ڵ�
    FTimerHandle MagicComboTimerHandle;


    // ����Ʈ�� ���� ��Ȯ�� ��ġ�� ����صδ� ����
    FVector CachedMagicLocation;

    // BeginPlay에서 한 번만 조회해 캐싱 — Tick마다 FindComponentByClass() 호출 방지
    UPROPERTY()
    UTargetingComponent* CachedTargetingComp;

    // BeginPlay에서 한 번만 찾아 캐싱 — WeaponTraceTick() 매 프레임 이름 검색 방지
    UPROPERTY()
    USkeletalMeshComponent* CachedWeaponMesh;

    // StartWeaponTrace() 호출 시 한 번만 조회 — WeaponTraceTick() 매 프레임 FindRow() 방지
    FAttackData* CachedAttackData = nullptr;





public:
    // �ִ� ü��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float MaxHP = 100.0f;

    // ���� ü��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float CurrentHP = 100.0f;

    // �ִ� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float MaxMP = 100.0f;

    // ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float CurrentMP = 100.0f;

    // �ʴ� ���� �ڿ� ȸ�� �ӵ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float ManaRegenRate = 5.0f;

    // ������ ���� �� ������ �Ǵ� �⺻ ���ݷ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float BaseAttackPower = 10.0f;

    // �����Ͽ� ĳ���Ϳ� ����� ���� Ŭ����
    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<AMySword> SwordClass;

    // ���� ����(����)�� ������ ���� ��Ÿ�� �ν��Ͻ� ������
    UPROPERTY(VisibleInstanceOnly, Category = "Combat")
    AMySword* CurrentWeapon;

    // ���� Į���� ���� ������ ������ �ý��������� Ȱ��ȭ���� ����
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bIsWeaponEnabled = false;

    // ���󿡼��� �Ϲ� ���� �� �޺��� ����ϴ� �ִϸ��̼� ��Ÿ��
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    UAnimMontage* ComboActionMontage;

    // ���߿��� Ÿ�� Ű �Է� �� ����� ���� ���� ��Ÿ��
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    UAnimMontage* JumpAttackMontage;

    // ���� ���� ���� �޺� ����
    int32 CurrentCombo = 0;

    // �ִ� ��ȯ ������ �޺� �Ѱ�ġ
    int32 MaxCombo = 3;

    // �ִϸ��̼� ��Ƽ���� ������ �̸� Ÿ���� ������ ���Է� ���� ����
    bool bIsComboInputOn = false;

    // Q ��ų ��� �� �ٽ� �����ϱ������ ���� �ð�(��)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float MaxCooldown_Q = 10.0f;

    // Q ��ų ���� �ܿ� ��ٿ�
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
    float CurrentCooldown_Q = 0.0f;

    // Q ��ų ������ ����� �ִϸ��̼� ��Ÿ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* SkillMontage;

    // ���� �� ���� �ڵ� ȸ��, ��ų Ÿ�̸� ó�� �� Ÿ�� �ڵ� �Ͽ� ȸ��
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ����� �Է��� ������ ��, ���� ����(����/����)�� �м��Ͽ� �˸��� ������ �õ�
    void Attack();

    // ���� ��� ���� �� ��Ÿ�� ������ �˻��� �� Q ��ų �õ�
    void SkillQ();

    // ���� �޺� �ִϸ��̼� ������ ���̸� ���
    void PlayComboAnim();

    // �ִϸ��̼� �̺�Ʈ(��Ƽ����) ���� ��, ���Է� ������ ���� �ִٸ� ���� �޺��� ����
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ProcessComboCommand();

    // ��Ÿ�� ����� �������Ǿ��� �� �׼� ���� ����(�±� ��)�� �ʱ�ȭ
    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // Ÿ�� Ÿ�� ���� ��/�Ŀ� ���� Ʈ���̽� ��� ���θ� ����
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetWeaponCollision(bool bEnable);

    // ���� ��Ȳ���� �÷��̾� ��Ʈ�ѷ��� Ʈ������ � �����ϱ� ���� ������ ������ ��ȯ
    AMyCharacter* GetCharacterOwner();

    // �� ��ȯ�� �Ͼ�� ���� ü��/������ ���ư��� �ʵ��� ����ý��ۿ� ���
    void UpdateStatToSubsystem();

    // �ܺ� ����(���� ���� ��)�� ���� ������ ü�� ȸ�� ��ġ
    UFUNCTION(BlueprintCallable, Category = "Combat|Stats")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Combat|Stats")
    void RestoreMana(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Combat|Stats")
    void IncreaseAttackPower(float Amount);

    // ���� ���� �ִϸ��̼� ��Ʈ ����
    void PlayJumpAttackAnim();
    // ���߿��� ���� ���������� ü���ð� ���� ��Ʈ ����
    void PlayJumpLoopAnim();
    // ���鿡 ����� �� �ļ� ��Ʈ ���� �� ��� ���
    void PlayJumpLandingAnim();

    // Ư�� ��Ÿ�� Ÿ�ֿ̹� ���� Ÿ�� ���� ������ ��ȭ
    void ExecuteAttackHit(FName HitAttackID);

    // ���� ���� ��� Ÿ�� ������ �Ѱ� �ߺ� Ÿ�� ���� ��� ����
    void StartWeaponTrace(FName HitAttackID);
    // ���� ���� ���� ������ ������ ������ ���� ���� ������ ������ Ʈ���̽� �߻�
    void WeaponTraceTick();
    // ���� ��� ������ ������ ������ 
    void EndWeaponTrace();

    // �ǰ� �� ���� ü�¿��� ���� ����
    void ReceiveDamage(float DamageAmount);

    // �ִ� ���¹̳�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxSP;

    // ���� ���¹̳�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentSP;

    // Ÿ���� ���� �� ��ǥ�� ���� ��ġ�� ���� ��������� ȸ���ϴ� ���� ��ġ
    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackTrackingSpeed = 8.0f;


    // ���� ���� �Է��� ó���ϴ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Combat|Magic")
    void RightClickMagicAttack();



    // 기본 공격 히트 이펙트 — Niagara (둘 중 하나만 할당하면 됨)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|VFX")
    class UNiagaraSystem* HitEffect;

    // 기본 공격 히트 이펙트 — Cascade (둘 중 하나만 할당하면 됨)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|VFX")
    class UParticleSystem* HitEffectCascade;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Magic|VFX")
    TArray<class UNiagaraSystem*> MagicExplosionVFXs;

    // 1. ��Ƽ����: ����Ʈ�� �̸� ��ȯ�ϴ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Combat|Magic")
    void SpawnMagicVFX();

    // 2. ��Ƽ����: ��¥ �������� �ִ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Combat|Magic")
    void ApplyMagicDamage();





    //  ���� ���ݷ� ������ ���� �ִ��� Ȯ���ϴ� '��ȣ��'
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Buff")
    bool bIsAttackBuffActive = false;

    //  ���߿� ������ ���� ��, ��Ȯ�� ��ŭ�� ���ݷ��� ���� �ϴ��� ����ص� ����
    float ActiveAttackBuffAmount = 0.0f;

    //  �𸮾� ������ �˶��ð� (Ÿ�̸�)
    FTimerHandle AttackBuffTimerHandle;

    //  ������ �Ѱ� ���� ���� �Լ�   
    void RemoveAttackBuff();


    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCombatBuffUpdated OnCombatBuffUpdated;

    UPROPERTY()
    FName ActiveAttackBuffID;

    // 아이템 버프 활성 여부 (스킬 버프와 독립적으로 중첩 가능)
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Buff")
    bool bIsItemAttackBuffActive = false;

    float ActiveItemAttackBuffAmount = 0.0f;
    FTimerHandle ItemAttackBuffTimerHandle;
    void RemoveItemAttackBuff();

    UPROPERTY()
    FName ActiveItemAttackBuffID;

    // bFromItem = true 이면 아이템 버프 경로, false 이면 스킬 버프 경로로 처리합니다.
    void ApplyAttackBuff(float Amount, float Duration, FName BuffID, bool bFromItem = false);
};