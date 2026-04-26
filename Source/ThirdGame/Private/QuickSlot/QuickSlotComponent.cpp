// =========================================================================================
// QuickSlotComponent.cpp
//
// [���� ���]
// ĳ����(����)�� �����Ǿ� ������ ����� ���� ���纻�� �����ϰ�, ����ý���(���� �����) �� UI ���� ����ȭ�� �߰��ϴ� ������Ʈ�Դϴ�.
// =========================================================================================

#include "QuickSlotComponent.h"
#include "QuickSlotSubsystem.h"

// ������Ʈ�� �ý��� ƽ ��Ȱ��ȭ �� �ʱ� ȯ���� �����մϴ�.
UQuickSlotComponent::UQuickSlotComponent()
{
	// ��� ������Ʈ ������ �ʿ����� �����Ƿ� �ڿ� ������ ���� ƽ�� ���ϴ�.
	PrimaryComponentTick.bCanEverTick = false;
}

// ���� ������ �������� ������ ����ý����� �����͸� ȹ���Ͽ� ��ȯ�մϴ�.
UQuickSlotSubsystem* UQuickSlotComponent::GetQuickSlotSubsystem()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return nullptr;

	return GI->GetSubsystem<UQuickSlotSubsystem>();
}

// ���� ���� �� ����ý����� ������ �����͸� ���ÿ� �����ϰ� �⺻ ���� ������ Ȯ���մϴ�.
void UQuickSlotComponent::BeginPlay()
{
	Super::BeginPlay();

	UQuickSlotSubsystem* Subsystem = GetQuickSlotSubsystem();

	// ����ý����� ã�� ���ϴ� ���� ��Ȳ�� ����� ���ÿ����� �⺻ 5ĭ�� �����մϴ�.
	if (!Subsystem)
	{
		Slots.SetNum(5);
		return;
	}

	// ���� ���� ���� Ȥ�� �迭�� �� ������ �� ũ���ø� ���� ���� �̸� 5ĭ�� ���� �����͸� ä���Ӵϴ�.
	if (Subsystem->QuickSlotContent.Num() < 5)
	{
		Subsystem->QuickSlotContent.SetNum(5);
	}

	Slots = Subsystem->QuickSlotContent;

	// UI�� �ʱ� �ε�� ���� �����͸� ��� �׷��� �� �ֵ��� �˸��ϴ�.
	if (OnQuickSlotUpdated.IsBound())
	{
		OnQuickSlotUpdated.Broadcast();
	}
}

// Ư�� ������ �ε����� ���ο� ������ �����͸� �����ϰ� UI ������ �����մϴ�.
void UQuickSlotComponent::SetSlot(int32 SlotIndex, const FItemData& NewItem)
{
	if (!Slots.IsValidIndex(SlotIndex)) return;

	UQuickSlotSubsystem* Subsystem = GetQuickSlotSubsystem();

	// �� ��ȯ ���Ŀ��� ���۵� ������ ������ �ʱ�ȭ���� �ʵ��� ���� ���� ����ҿ��� ���� �����ϴ�.
	if (Subsystem && Subsystem->QuickSlotContent.IsValidIndex(SlotIndex))
	{
		Subsystem->QuickSlotContent[SlotIndex] = NewItem;
	}

	Slots[SlotIndex] = NewItem;

	//UE_LOG(LogTemp, Warning, TEXT("������ %d���� [%s] ��ϵ�!"), SlotIndex + 1, *NewItem.ItemName);

	if (OnQuickSlotUpdated.IsBound())
	{
		OnQuickSlotUpdated.Broadcast();
	}
}

// ������ �ε����� �����Կ� ��ϵ� ������ ���(�Ҹ�) ó���� �����մϴ�.
void UQuickSlotComponent::UseSlot(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex)) return;
	if (Slots[SlotIndex].ItemIcon == nullptr) return;

	//UE_LOG(LogTemp, Warning, TEXT("������ %d�� ���: �ܲ�! [%s]"), SlotIndex + 1, *Slots[SlotIndex].ItemName);
}