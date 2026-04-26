// =========================================================================================
// StatSubsystem.cpp
//
// [파일 역할]
// 레벨 전환 시 캐릭터의 HP·MP·SP 수치를 유지하기 위한 GameInstance 서브시스템입니다.
// CombatComponent는 BeginPlay 시 이 서브시스템에서 스탯을 불러오고,
// EndPlay 또는 수치 변경 시 여기에 저장합니다.
// SavedCurrentHP가 -1이면 '저장된 값 없음(최초 실행)'을 의미합니다.
// =========================================================================================

#include "StatSubsystem.h"

// 서브시스템 초기화 시 저장 슬롯을 '미저장' 상태(-1)와 기본 최대치(100)로 설정합니다.
void UStatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SavedCurrentHP = -1.0f;  // -1 = 저장된 값 없음 (CombatComponent가 기본값을 사용)
	SavedCurrentMP = -1.0f;
	SavedCurrentSP = -1.0f;

	SavedMaxHP = 100.0f;
	SavedMaxMP = 100.0f;
	SavedMaxSP = 100.0f;
}
