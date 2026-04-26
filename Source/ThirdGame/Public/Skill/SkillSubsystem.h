// =========================================================================================
// SkillSubsystem.h
//
// [���� ���]
// ���� �������� ��ų ���� �����͸� �����ϰ�, ������ ���� ���� �� �� �̵� �� �÷��̾��� ��Ÿ��/���� ���¸� ���� �� �����ϴ� ����ý��� ����Դϴ�.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SkillData.h" 
#include "SkillComponent.h"
#include "SkillSubsystem.generated.h"

// ��ų ����Ű ���� ���°� �����Ǿ��� �� UI ȭ�� ���ΰ�ħ�� �����ϱ� ���� ��������Ʈ�Դϴ�.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillSlotUpdatedSignature);

UCLASS()
class THIRDGAME_API USkillSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USkillSubsystem();

	// ������ ��ų ID �̸��� �ش��ϴ� ���� �� ���� �����͸� ������ ���̺����� ��ȸ�Ͽ� ��ȯ�մϴ�.
	FSkillData* GetSkillData(FName SkillID);

	// ��� ������ ��ȣ ��ġ�� ����ڰ� ������ Ư�� ��ų�� ���Ӱ� ����(����)�մϴ�.
	UFUNCTION(BlueprintCallable)
	void EquipSkill(int32 SlotIndex, FName SkillID);

	// �ش� ������ ��ȣ�� ���� �����Ǿ� �ִ� ��ų�� ���� ID�� ��ȯ�մϴ�.
	UFUNCTION(BlueprintCallable)
	FName GetSkillIDInSlot(int32 SlotIndex);

	// �ý��ۿ� �ε�Ǿ� �ִ� ��ü ��ų ������ ���̺� �� ��ü�� �ܺ� �ý������� ��ȯ�մϴ�.
	UDataTable* GetSkillDataTable() const { return SkillDataTable; }

	// ���� ���� ���� ���� �� �뺸�� �ޱ� ���� ��������Ʈ �̺�Ʈ �ν��Ͻ��Դϴ�.
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSkillSlotUpdatedSignature OnSkillSlotUpdated;


	// 레벨 이동 전 SkillComponent 상태(스킬 버프/쿨다운)를 임시 저장합니다.
	void SaveTemporaryData(const TMap<FName, float>& InCooldowns, const TArray<FActiveBuff>& InBuffs);

	// 새 레벨에서 SkillComponent가 복원되었을 때 저장된 스킬 쿨다운과 버프 상태를 복원합니다.
	void LoadTemporaryData(TMap<FName, float>& OutCooldowns, TArray<FActiveBuff>& OutBuffs);

	// 레벨 이동 전 버프 아이템(공격력 버프) 상태를 임시 저장합니다.
	void SaveAttackBuffData(bool bActive, FName BuffID, float Amount, float InRemainingTime);

	// 새 레벨에서 버프 아이템 상태를 복원합니다. 로드 후 백업은 초기화됩니다.
	void LoadAttackBuffData(bool& bActive, FName& BuffID, float& Amount, float& OutRemainingTime);

protected:
	// 스킬의 전체 목록을 담고 있는 스킬 데이터 테이블 레퍼런스입니다.
	UPROPERTY()
	UDataTable* SkillDataTable;

	// 각 빠른 슬롯 키인덱스에 대응하는 스킬 이름과 함께 저장된 빠른 슬롯 할당 매핑입니다.
	UPROPERTY()
	TMap<int32, FName> QuickSkillSlots;

	// 레벨 이동 시 보존하는 스킬 쿨다운 백업
	UPROPERTY()
	TMap<FName, float> BackupCooldowns;

	// 레벨 이동 시 보존하는 스킬 버프 백업
	UPROPERTY()
	TArray<FActiveBuff> BackupBuffs;

	// ===== 버프 아이템(공격력 버프) 백업 =====

	UPROPERTY()
	bool BackupAttackBuffActive = false;

	UPROPERTY()
	FName BackupAttackBuffID;

	UPROPERTY()
	float BackupAttackBuffAmount = 0.0f;

	UPROPERTY()
	float BackupAttackBuffRemainingTime = 0.0f;
};