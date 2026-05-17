// =========================================================================================
// QuickSlotComponent.cpp
//
// [파일 역할]
// 캐릭터(플레이어)에 부착되어 퀵슬롯 데이터의 로컬 복사본을 관리하고,
// 서브시스템(전역 데이터) 및 UI 갱신 연동을 담당하는 컴포넌트입니다.
// =========================================================================================

#include "QuickSlotComponent.h"
#include "QuickSlotSubsystem.h"

// 컴포넌트의 시스템 틱 비활성화 및 초기 환경을 설정합니다.
UQuickSlotComponent::UQuickSlotComponent()
{
	// 이 컴포넌트 작동에 틱이 필요없으므로 자원과 성능을 위해 틱을 끕니다.
	PrimaryComponentTick.bCanEverTick = false;
}

// 현재 월드의 게임인스턴스를 통해 퀵슬롯 서브시스템의 데이터를 획득하여 반환합니다.
UQuickSlotSubsystem* UQuickSlotComponent::GetQuickSlotSubsystem()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return nullptr;

	return GI->GetSubsystem<UQuickSlotSubsystem>();
}

// 게임 시작 시 서브시스템의 퀵슬롯 데이터를 로드하고 기본 슬롯 개수를 확인합니다.
void UQuickSlotComponent::BeginPlay()
{
	Super::BeginPlay();

	UQuickSlotSubsystem* Subsystem = GetQuickSlotSubsystem();

	// 서브시스템을 찾지 못하는 경우에 대비해 로컬에서만 기본 5칸을 설정합니다.
	if (!Subsystem)
	{
		Slots.SetNum(5);
		return;
	}

	// 퀵슬롯 배열의 크기가 최솟값보다 작을 경우 부족한 만큼 기본 데이터로 채웁니다.
	if (Subsystem->QuickSlotContent.Num() < 5)
	{
		Subsystem->QuickSlotContent.SetNum(5);
	}

	Slots = Subsystem->QuickSlotContent;

	// UI의 초기 로드를 위해 현재 데이터를 모두 표시할 수 있도록 알립니다.
	if (OnQuickSlotUpdated.IsBound())
	{
		OnQuickSlotUpdated.Broadcast();
	}
}

// 특정 슬롯의 인덱스에 새로운 아이템 데이터를 할당하고 UI 갱신을 처리합니다.
void UQuickSlotComponent::SetSlot(int32 SlotIndex, const FItemData& NewItem)
{
	if (!Slots.IsValidIndex(SlotIndex)) return;

	UQuickSlotSubsystem* Subsystem = GetQuickSlotSubsystem();

	// 로컬 교환 이후에도 전역 서브시스템 데이터가 동기화되도록 서브시스템에도 반영합니다.
	if (Subsystem && Subsystem->QuickSlotContent.IsValidIndex(SlotIndex))
	{
		Subsystem->QuickSlotContent[SlotIndex] = NewItem;
	}

	Slots[SlotIndex] = NewItem;

	//UE_LOG(LogTemp, Warning, TEXT("퀵슬롯 %d번에 [%s] 등록됨!"), SlotIndex + 1, *NewItem.ItemName);

	if (OnQuickSlotUpdated.IsBound())
	{
		OnQuickSlotUpdated.Broadcast();
	}
}

// 슬롯의 인덱스에 해당하는 등록된 아이템의 사용(소모) 처리를 담당합니다.
void UQuickSlotComponent::UseSlot(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex)) return;
	if (Slots[SlotIndex].ItemIcon == nullptr) return;

	//UE_LOG(LogTemp, Warning, TEXT("퀵슬롯 %d번 사용: 성공! [%s]"), SlotIndex + 1, *Slots[SlotIndex].ItemName);
}
