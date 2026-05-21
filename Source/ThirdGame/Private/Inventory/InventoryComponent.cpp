// =========================================================================================
// InventoryComponent.cpp
//
// [파일 역할]
// 플레이어(폰)에 부착되어 인벤토리 데이터를 서버 권위로 관리하는 컴포넌트입니다.
// 실제 아이템 배열(Content)은 이 컴포넌트가 서버에서 소유하며, 소유 클라이언트에 복제됩니다.
// InventorySubsystem은 UI 표시용 미러로만 사용됩니다.
// =========================================================================================

#include "Inventory/InventoryComponent.h"
#include "Inventory/InventorySubsystem.h"
#include "QuickSlot/QuickSlotSubsystem.h"
#include "MyCharacter.h"
#include "Item/ItemEffectBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// 인벤토리 관련 로직에는 Tick이 필요하지 않으므로 비활성화합니다.
// SetIsReplicated(true)로 컴포넌트 복제를 활성화합니다.
UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
}

// 복제할 변수를 등록합니다. COND_OwnerOnly로 소유 클라이언트에만 전송해 대역폭을 절약합니다.
void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UInventoryComponent, Content, COND_OwnerOnly);
}

// BeginPlay에서 Content 배열을 Capacity 크기만큼 빈 슬롯으로 초기화합니다.
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 Content 배열을 초기화합니다. 클라이언트는 복제를 통해 받습니다.
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (Content.Num() < Capacity)
		{
			Content.SetNum(Capacity);
		}
	}
}

// InventorySubsystem 레퍼런스를 반환하는 내부 헬퍼 함수입니다.
UInventorySubsystem* UInventoryComponent::GetInventorySubsystem()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return nullptr;
	return GI->GetSubsystem<UInventorySubsystem>();
}

// Content 복제 수신 시 호출 — InventorySubsystem에 동기화하고 UI 갱신 델리게이트를 브로드캐스트합니다.
// 기존 UI 코드(InventoryWidget, QuickSlotSubsystem 등)가 InventorySubsystem을 읽으므로 여기서 동기화합니다.
void UInventoryComponent::OnRep_Inventory()
{
	UInventorySubsystem* InvSys = GetInventorySubsystem();
	if (!InvSys) return;

	// 서버로부터 받은 Content를 InventorySubsystem에 그대로 복사합니다.
	InvSys->Content = Content;
	InvSys->Capacity = Capacity;

	int32 ValidCount = 0;
	for (const FItemData& Item : Content)
	{
		if (Item.ItemIcon != nullptr) ValidCount++;
	}
	UE_LOG(LogTemp, Log, TEXT("[INVEN_SYNC][CLIENT][%s] OnRep received -> InventorySubsystem synced (valid items: %d)"), *GetOwner()->GetName(), ValidCount);

	// 기존 UI 갱신 델리게이트를 브로드캐스트합니다. QuickSlotSubsystem 동기화도 자동으로 호출됩니다.
	if (InvSys->OnInventoryUpdated.IsBound())
	{
		InvSys->OnInventoryUpdated.Broadcast();
	}
}

// 실제 아이템 추가 로직입니다. 서버에서만 호출합니다.
// 중첩 가능하면 기존 슬롯에 합치고, 아니면 빈 슬롯을 찾아 배치합니다.
bool UInventoryComponent::AddItemInternal(const FItemData& NewItem)
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

				ExistingItem.Quantity += AmountToAdd;
				IncomingItem.Quantity -= AmountToAdd;

				if (IncomingItem.Quantity <= 0)
				{
					UE_LOG(LogTemp, Log, TEXT("[INVEN_SYNC][SERVER][%s] AddItem: %s +%d stacked (total: %d)"),
						*GetOwner()->GetName(), *ExistingItem.ItemName, AmountToAdd, ExistingItem.Quantity);
					// Content가 변경되었으므로 복제가 자동으로 트리거됩니다.
					return true;
				}
			}
		}
	}

	if (IncomingItem.Quantity <= 0) return true;

	// ItemIcon == nullptr을 빈 슬롯 기준으로 사용합니다.
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
		UE_LOG(LogTemp, Warning, TEXT("[INVEN_SYNC][SERVER][%s] AddItem FAILED: %s - inventory full"),
			*GetOwner()->GetName(), *IncomingItem.ItemName);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[INVEN_SYNC][SERVER][%s] AddItem: %s x%d -> slot %d"),
		*GetOwner()->GetName(), *IncomingItem.ItemName, IncomingItem.Quantity, EmptySlotIndex);
	Content[EmptySlotIndex] = IncomingItem;
	return true;
}

// 실제 슬롯 교환 로직입니다. 서버에서만 호출합니다.
void UInventoryComponent::SwapItemsInternal(int32 IndexA, int32 IndexB)
{
	if (!Content.IsValidIndex(IndexA) || !Content.IsValidIndex(IndexB)) return;
	if (IndexA == IndexB) return;

	Content.Swap(IndexA, IndexB);
}

// 서버에서 실행: 아이템 추가를 검증하고 Content에 반영합니다.
void UInventoryComponent::ServerAddItem_Implementation(FItemData NewItem)
{
	AddItemInternal(NewItem);
	// Content가 Replicated이므로 변경 시 OnRep_Inventory가 소유 클라이언트에서 자동 호출됩니다.
}

// 서버에서 실행: 두 슬롯을 교환하고 복제합니다.
void UInventoryComponent::ServerSwapItems_Implementation(int32 IndexA, int32 IndexB)
{
	UE_LOG(LogTemp, Log, TEXT("[INVEN_SYNC][SERVER][%s] SwapItems: slot %d <-> slot %d"), *GetOwner()->GetName(), IndexA, IndexB);
	SwapItemsInternal(IndexA, IndexB);
}

// 서버에서 실행: GUID로 아이템을 찾아 효과를 적용하고 수량을 감소시킵니다.
void UInventoryComponent::ServerUseItem_Implementation(FGuid ItemID, int32 QuickSlotIndex)
{
	if (!ItemID.IsValid()) return;

	AMyCharacter* MyChar = Cast<AMyCharacter>(GetOwner());
	if (!MyChar) return;

	for (FItemData& InvItem : Content)
	{
		if (InvItem.ItemID != ItemID) continue;
		if (InvItem.Quantity <= 0) break;
		if (!InvItem.ItemEffectClass) break;

		UItemEffectBase* ItemEffect = NewObject<UItemEffectBase>(this, InvItem.ItemEffectClass);
		if (!ItemEffect) break;

		// 사용 가능 조건을 검사합니다. (HP 최대치, 버프 중복 등)
		if (!ItemEffect->CanUseItem(MyChar)) break;

		// 효과를 적용합니다. (HP/MP/SP 회복, 버프 등 — 서버 권위 스탯에 직접 적용)
		ItemEffect->ExecuteItemEffect(MyChar);

		// 수량을 1 감소합니다.
		FString UsedItemName = InvItem.ItemName;
		InvItem.Quantity--;
		if (InvItem.Quantity <= 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[INVEN_SYNC][SERVER][%s] UseItem: %s depleted -> slot cleared"), *GetOwner()->GetName(), *UsedItemName);
			InvItem = FItemData();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[INVEN_SYNC][SERVER][%s] UseItem: %s used, remaining: %d"), *GetOwner()->GetName(), *UsedItemName, InvItem.Quantity);
		}

		// Content가 변경되었으므로 소유 클라이언트에 자동 복제됩니다.
		// QuickSlotSubsystem의 SyncQuickSlotsWithInventory는 OnRep_Inventory → OnInventoryUpdated 브로드캐스트로 자동 호출됩니다.
		break;
	}
}
