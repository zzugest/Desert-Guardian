#include "Skill/SkillSubsystem.h"
#include "UObject/ConstructorHelpers.h"

// =========================================================================================
// SkillSubsystem.cpp
//
// [파일 역할]
// 게임 인스턴스 전체에서 스킬 데이터(데이터테이블)를 관리하고 저장하는 서브시스템입니다.
// 스킬 슬롯 변경 및 중복 방지, OnSkillSlotUpdated 브로드캐스트를 담당합니다.
// =========================================================================================

USkillSubsystem::USkillSubsystem()
{
	// 생성자에서 스킬 데이터테이블을 하드코딩 경로로 로드합니다. (경로가 바뀌면 수정 필요)
	static ConstructorHelpers::FObjectFinder<UDataTable> DT_Asset(TEXT("/Script/Engine.DataTable'/Game/Skill/DT_SkillData.DT_SkillData'"));

	if (DT_Asset.Succeeded())
	{
		SkillDataTable = DT_Asset.Object;
	}
}

FSkillData* USkillSubsystem::GetSkillData(FName SkillID)
{
	// 데이터테이블이 없으면 nullptr 반환
	if (!SkillDataTable) return nullptr;

	// 데이터테이블에서 해당 Row(SkillID)를 찾아 반환
	return SkillDataTable->FindRow<FSkillData>(SkillID, TEXT("GetSkillData"));
}

void USkillSubsystem::EquipSkill(int32 SlotIndex, FName SkillID)
{
	// 1) 다른 슬롯들에서 같은 SkillID가 이미 등록된 슬롯을 찾아 중복 장착 방지
	TArray<int32> DuplicateSlots;
	for (const auto& Pair : QuickSkillSlots)
	{
		if (Pair.Value == SkillID)
		{
			DuplicateSlots.Add(Pair.Key);
		}
	}

	// 2) 중복 슬롯을 빈 값(NAME_None)으로 초기화 (기존 슬롯 해제)
	for (int32 DuplicateIndex : DuplicateSlots)
	{
		QuickSkillSlots.Add(DuplicateIndex, NAME_None);
	}

	// 3) 새 슬롯에 스킬 장착
	QuickSkillSlots.Add(SlotIndex, SkillID);

	// 슬롯 변경 알림 브로드캐스트
	OnSkillSlotUpdated.Broadcast();
}

FName USkillSubsystem::GetSkillIDInSlot(int32 SlotIndex)
{
	// Map에 키가 있으면 반환, 없으면 빈 이름 반환
	if (QuickSkillSlots.Contains(SlotIndex))
	{
		return QuickSkillSlots[SlotIndex];
	}

	return FName();
}

void USkillSubsystem::SaveTemporaryData(const TMap<FName, float>& InCooldowns, const TArray<FActiveBuff>& InBuffs)
{
	// 레벨 전환/맵 이동 시 SkillComponent 상태를 임시 저장하기 위한 함수
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