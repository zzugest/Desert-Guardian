#include "Skill/SkillSubsystem.h"
#include "UObject/ConstructorHelpers.h"

// SkillSubsystem.cpp
// Purpose:
//   - ������Ʈ �� ��ų ������(���������̺�) ���� �� ������ ���� ���� ���� ����.
//   - ��ų ���� ������ �ߺ� ���� �� OnSkillSlotUpdated ��ε�ĳ��Ʈ ����.
// Key behaviors:
//   - �����ڿ��� ���������̺� �ε�.
//   - GetSkillData: SkillID�� ������ ��ȸ.
//   - EquipSkill: ���Կ� ��ų ���(�ߺ� ���� ���� ����).
//   - �ӽ� ������ ���/���� API: SaveTemporaryData / LoadTemporaryData.
// Safety notes:
//   - ConstructorHelpers�� �ϵ��ڵ��� ��ΰ� ��Ȯ���� �����Ϳ��� Ȯ�� �ʿ�.
//   - QuickSkillSlots�� TMap�̹Ƿ� Add�� ����� ������ ��.

USkillSubsystem::USkillSubsystem()
{
	// �����Ϳ��� ���� ���������̺��� �ϵ� ��η� �ε� (���� ���� ��ΰ� �ٲ�� ������)
	static ConstructorHelpers::FObjectFinder<UDataTable> DT_Asset(TEXT("/Script/Engine.DataTable'/Game/Skill/DT_SkillData.DT_SkillData'"));

	if (DT_Asset.Succeeded())
	{
		SkillDataTable = DT_Asset.Object;
	}
}

FSkillData* USkillSubsystem::GetSkillData(FName SkillID)
{
	// ���������̺��� ������ nullptr ��ȯ
	if (!SkillDataTable) return nullptr;

	// ���������̺����� �ش� Row(SkillID)�� ã�� ��ȯ
	return SkillDataTable->FindRow<FSkillData>(SkillID, TEXT("GetSkillData"));
}

void USkillSubsystem::EquipSkill(int32 SlotIndex, FName SkillID)
{
	// 1) ���� ���Ե鿡�� ���� SkillID�� �̹� ��ϵ� ���Ե��� �����Ͽ� �ߺ� ���� �غ�
	TArray<int32> DuplicateSlots;
	for (const auto& Pair : QuickSkillSlots)
	{
		if (Pair.Value == SkillID)
		{
			DuplicateSlots.Add(Pair.Key);
		}
	}

	// 2) �ߺ� ������ �� ����(NAME_None)���� �ʱ�ȭ(�Ǵ� ���� ����)
	for (int32 DuplicateIndex : DuplicateSlots)
	{
		QuickSkillSlots.Add(DuplicateIndex, NAME_None);
	}

	// 3) �� ���Կ� �����
	QuickSkillSlots.Add(SlotIndex, SkillID);

	// ���� �˸�
	OnSkillSlotUpdated.Broadcast();
}

FName USkillSubsystem::GetSkillIDInSlot(int32 SlotIndex)
{
	// Map�� Ű�� ������ ���� ��ȯ, ������ �� �̸� ��ȯ
	if (QuickSkillSlots.Contains(SlotIndex))
	{
		return QuickSkillSlots[SlotIndex];
	}

	return FName();
}

void USkillSubsystem::SaveTemporaryData(const TMap<FName, float>& InCooldowns, const TArray<FActiveBuff>& InBuffs)
{
	// ���� ��ȯ/�� �̵� �� SkillComponent ���¸� �ӽ� �����ϱ� ���� ���
	BackupCooldowns = InCooldowns;
	BackupBuffs = InBuffs;
}

void USkillSubsystem::LoadTemporaryData(TMap<FName, float>& OutCooldowns, TArray<FActiveBuff>& OutBuffs)
{
	OutCooldowns = BackupCooldowns;
	OutBuffs = BackupBuffs;
}

void USkillSubsystem::SaveAttackBuffData(bool bActive, FName BuffID, float Amount, float InRemainingTime)
{
	BackupAttackBuffActive = bActive;
	BackupAttackBuffID = BuffID;
	BackupAttackBuffAmount = Amount;
	BackupAttackBuffRemainingTime = InRemainingTime;
}

void USkillSubsystem::LoadAttackBuffData(bool& bActive, FName& BuffID, float& Amount, float& OutRemainingTime)
{
	bActive = BackupAttackBuffActive;
	BuffID = BackupAttackBuffID;
	Amount = BackupAttackBuffAmount;
	OutRemainingTime = BackupAttackBuffRemainingTime;

	// 로드 후 백업 초기화 (중복 적용 방지)
	BackupAttackBuffActive = false;
	BackupAttackBuffRemainingTime = 0.0f;
}