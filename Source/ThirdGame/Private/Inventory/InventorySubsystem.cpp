// =========================================================================================
// InventorySubsystem.cpp
//
// [파일 역할]
// 게임 전체에서 플레이어의 인벤토리 데이터를 저장하고 관리하는 싱글턴 서브시스템입니다.
// 아이템 획득(ID 발급 및 중첩 처리), 슬롯 비어있는 여부 판단, 인덱스 간 위치 교환 기능을 제공합니다.
// 변경이 발생할 때마다 OnInventoryUpdated 델리게이트를 브로드캐스트해 UI 갱신을 알립니다.
// =========================================================================================

#include "Inventory/InventorySubsystem.h"

// Capacity 기본값은 헤더의 UPROPERTY 선언부(= 20)에서 설정합니다.
UInventorySubsystem::UInventorySubsystem()
{
}

// 서브시스템 초기화 시 Content 배열을 Capacity 크기만큼 빈 슬롯으로 미리 채웁니다.
void UInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 인덱스 기반 접근과 드래그 앤 드롭 교환이 올바르게 동작하도록 배열을 고정 크기로 유지합니다.
    if (Content.Num() < Capacity)
    {
        Content.SetNum(Capacity);
    }
}

// 새 아이템을 인벤토리에 추가합니다. 중첩 가능하면 기존 슬롯에 합치고, 아니면 빈 슬롯을 찾아 배치합니다.
bool UInventorySubsystem::AddItem(const FItemData& NewItem)
{
    FItemData IncomingItem = NewItem;

    // 드래그·교환·저장 시 아이템을 고유하게 식별하기 위해 GUID를 발급합니다.
    if (!IncomingItem.ItemID.IsValid())
    {
        IncomingItem.ItemID = FGuid::NewGuid();
    }

    // 중첩 가능 아이템은 같은 이름의 슬롯에 먼저 채워 넣습니다.
    if (IncomingItem.bIsStackable)
    {
        for (FItemData& ExistingItem : Content)
        {
            if (ExistingItem.ItemName == IncomingItem.ItemName && ExistingItem.Quantity < ExistingItem.MaxStackSize)
            {
                int32 SpaceLeft   = ExistingItem.MaxStackSize - ExistingItem.Quantity;
                int32 AmountToAdd = FMath::Min(SpaceLeft, IncomingItem.Quantity);

                ExistingItem.Quantity  += AmountToAdd;
                IncomingItem.Quantity  -= AmountToAdd;

                // 남은 수량이 없으면 추가 완료입니다.
                if (IncomingItem.Quantity <= 0)
                {
                    if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast();
                    return true;
                }
            }
        }
    }

    if (IncomingItem.Quantity <= 0) return true;

    // ItemIcon == nullptr을 빈 슬롯 기준으로 사용합니다.
    // InventorySlotWidget의 SetItem()도 동일한 기준을 사용해 일관성을 유지합니다.
    int32 EmptySlotIndex = -1;
    for (int32 i = 0; i < Content.Num(); i++)
    {
        if (Content[i].ItemIcon == nullptr)
        {
            EmptySlotIndex = i;
            break;
        }
    }

    // 빈 슬롯이 없으면 인벤토리가 가득 찬 것으로 처리합니다.
    if (EmptySlotIndex == -1)
    {
        return false;
    }

    Content[EmptySlotIndex] = IncomingItem;

    if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast();

    return true;
}

// 두 인덱스의 슬롯 데이터를 교환하고 UI 갱신을 알립니다 (드래그 앤 드롭 정렬에 사용).
void UInventorySubsystem::SwapItems(int32 IndexA, int32 IndexB)
{
    if (!Content.IsValidIndex(IndexA) || !Content.IsValidIndex(IndexB)) return;

    if (IndexA == IndexB) return;

    Content.Swap(IndexA, IndexB);

    if (OnInventoryUpdated.IsBound())
    {
        OnInventoryUpdated.Broadcast();
    }
}
