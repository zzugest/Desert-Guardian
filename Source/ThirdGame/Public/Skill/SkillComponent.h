// =========================================================================================
// SkillComponent.h
//
// [���� ���]
// ĳ���Ϳ� �����Ǿ� ��ų ���� �� ����/��ٿ� ����, ������ ��� �׼� ����, �׸��� �÷��̾�� ����� ���� ���¸� �����ϴ� ������Ʈ ����Դϴ�.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"

// ���� ����� ���ŵǾ��� �� ȭ��(UI)�� Ÿ�̸ӿ� ������ ���ΰ�ħ�� �����ϱ� ���� ��������Ʈ�Դϴ�.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuffListUpdatedSignature);

// ���� ĳ���Ϳ� Ȱ��ȭ�� ���� ������ ������(�ð�)�� ����ϴ� ��ο� ����ü�Դϴ�.
USTRUCT(BlueprintType)
struct FActiveBuff
{
	GENERATED_BODY()

	// ���� ���� ������ ��Ī/���� �ĺ����Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName BuffID;

	// �ش� ������ �Ҹ��ϱ���� ���� �ð�(��)�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RemainingTime;

	// Ÿ�̸� UI ���� ��� ���� ���� �⺻ �ο� �� ���� �ð��Դϴ�.
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
	// ������Ʈ �ʱ�ȭ �� ���� ó���� ���� ���� ������Ʈ ĳ���� �����մϴ�.
	virtual void BeginPlay() override;

	// ���� ��ȯ�̳� �� �̵� �� �����Ǿ�� �� ��ٿ�/���� ���¸� ���(����)�մϴ�.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// �� ������ ��ų ��ٿ�� Ȱ��ȭ�� ������ �ܿ� �ð��� ���̰� ���� ó���� �����մϴ�.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ��ٿ�� �ܿ� ������ �˻��� �� ������ ���� ��ȣ�� ��ų ������ �õ��մϴ�.
	UFUNCTION(BlueprintCallable)
	void TryCastSkill(int32 SlotIndex);

	// Ư�� ��ų�� �ٽ� ��� ������������� ���� �ð�(��)�� ��ȯ�մϴ�.
	UFUNCTION(BlueprintCallable)
	float GetRemainingCooldown(FName SkillID);

	// ������ ���̺����� Ư�� ��ų�� �ִ� ���� ���ð�(���� ��Ÿ��)�� ��ȸ�Ͽ� ��ȯ�մϴ�.
	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetMaxCooldown(FName SkillID) const;

	// ��ų ���� ��ȣ(Q, E ��)�� �ش��ϴ� ��ų ID�� ����صδ� ���� ����Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<int32, FName> QuickSlots;

	// Ư�� ���� �ε����� �÷��̾ ����� ��� ��ų(ID)�� �����Ͽ� ����մϴ�.
	UFUNCTION(BlueprintCallable)
	void RegistSkillToSlot(int32 SlotIndex, FName SkillID);

	// ��� ���� �ε����� ��ϵǾ� �ִ� ��ų�� ���� ID�� ��ȯ�մϴ�.
	UFUNCTION(BlueprintCallable)
	FName GetSkillIDAtSlot(int32 SlotIndex);

	// =============================================================
	// ���� �ý��� ���� �� �̺�Ʈ
	// =============================================================

	// ���� ĳ���Ϳ��� ȿ���� ���� ���� ��Ƽ�� �������� ���� ����Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buff")
	TArray<FActiveBuff> ActiveBuffs;

	// ������ �߰��ǰų� ���ŵ� �� UI ������ �����ϱ� ���� �̺�Ʈ �뺸���Դϴ�.
	UPROPERTY(BlueprintAssignable, Category = "Buff")
	FOnBuffListUpdatedSignature OnBuffListUpdated;

	// =============================================================
	// ���� �ý��� �Լ�
	// =============================================================

	// ��� ������ Ȱ�� ��Ͽ� �߰��ϰų� �̹� ���� ��� ���� �ð��� �ִ�ġ�� �����մϴ�.
	UFUNCTION(BlueprintCallable, Category = "Buff")
	void AddBuff(FName NewBuffID, float Duration);

	// ��ų �ִϸ��̼��� ��Ƽ����(Ư�� ���) Ÿ�ֿ̹� ���� ��ų ����ü(���� ��)�� �����մϴ�.
	void SpawnProjectile(FName SkillID, FName SocketName);

private:
	// SkillMontage가 끝났을 때 MagicCasting 상태 태그를 제거합니다.
	UFUNCTION()
	void OnSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// ���� ���ð��� ���� ����(��ٿ�) ��ų���� ���� �ð��� �����ϴ� �ý��� Ÿ�̸� ����Դϴ�.
	TMap<FName, float> ActiveCooldowns;

	// ��ų ��� �� �䱸 �������� �����ϱ� ���� ������ �� ���� ������Ʈ �������Դϴ�.
	UPROPERTY()
	UCombatComponent* CombatComp;
};