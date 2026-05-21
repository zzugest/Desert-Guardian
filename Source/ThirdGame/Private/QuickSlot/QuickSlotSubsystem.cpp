// =========================================================================================
// QuickSlotSubsystem.cpp
//
// [파일 역할]
// 퀵슬롯 데이터(최대 5칸)를 전역으로 관리하는 GameInstance 서브시스템입니다.
// 주요 기능:
//   - RegisterQuickSlot : 인벤토리 아이템을 특정 슬롯에 등록 (중복 슬롯 자동 정리)
//   - UseQuickSlot      : 슬롯 아이템을 소모하고 효과 적용 (쿨다운·사용 조건 검사)
//   - SwapQuickSlots    : 두 슬롯 간 위치 교환 (드래그 앤 드롭 정렬)
//   - SyncQuickSlotsWithInventory : 인벤토리 변경 시 퀵슬롯 수량·유효성 자동 동기화
// 쿨다운은 FPlatformTime 기반 절대 시각으로 관리해 타이머 오브젝트 없이 처리합니다.
// =========================================================================================

#include "QuickSlot/QuickSlotSubsystem.h"
#include "Inventory/InventorySubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Item/ItemEffectBase.h"
#include "MyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GlobalUI/WarningSubsystem.h"

// 서브시스템 초기화 시 InventorySubsystem의 OnInventoryUpdated 델리게이트를 구독합니다.
// 인벤토리가 변경될 때마다 퀵슬롯 수량·유효성을 자동으로 동기화합니다.
void UQuickSlotSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UInventorySubsystem* InvenSystem = Collection.InitializeDependency<UInventorySubsystem>();
    if (!InvenSystem) return;

    InvenSystem->OnInventoryUpdated.AddDynamic(this, &UQuickSlotSubsystem::SyncQuickSlotsWithInventory);
}

// 지정 슬롯 인덱스에 외부 아이템 데이터를 등록합니다.
// 같은 아이템이 다른 슬롯에 이미 등록되어 있으면 그 슬롯을 먼저 비웁니다(중복 방지).
void UQuickSlotSubsystem::RegisterQuickSlot(int32 SlotIndex, const FItemData& InItemData)
{
    if (!QuickSlotContent.IsValidIndex(SlotIndex)) return;

    FGuid NewItemID = InItemData.ItemID;

    if (NewItemID.IsValid())
    {
        // 동일 GUID의 아이템이 다른 슬롯에 있으면 해당 슬롯을 빈 칸으로 초기화합니다.
        for (int32 i = 0; i < QuickSlotContent.Num(); i++)
        {
            if (i == SlotIndex) continue;

            if (QuickSlotContent[i].ItemID == NewItemID)
            {
                QuickSlotContent[i] = FItemData();
            }
        }
    }

    QuickSlotContent[SlotIndex] = InItemData;

    if (OnQuickSlotUpdated.IsBound())
    {
        OnQuickSlotUpdated.Broadcast();
    }
}

// 지정 슬롯의 아이템을 사용합니다.
// 쿨다운·사용 조건은 클라이언트에서 선검사하고, 실제 수량 감소와 효과 적용은 서버 RPC로 처리합니다.
void UQuickSlotSubsystem::UseQuickSlot(int32 SlotIndex)
{
    if (!QuickSlotContent.IsValidIndex(SlotIndex)) return;

    FGuid TargetID = QuickSlotContent[SlotIndex].ItemID;
    if (!TargetID.IsValid()) return;

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UInventorySubsystem* InvenSystem = GI->GetSubsystem<UInventorySubsystem>();
    if (!InvenSystem) return;

    // 인벤토리에서 GUID로 아이템을 찾아 클라이언트 선검사를 수행합니다.
    for (const FItemData& InvItem : InvenSystem->Content)
    {
        if (InvItem.ItemID != TargetID) continue;
        if (InvItem.Quantity <= 0) return;
        if (!InvItem.ItemEffectClass) return;

        // 쿨다운이 남아 있으면 남은 시간을 경고창으로 표시하고 사용을 막습니다.
        float TimeLeft = GetRemainingCooldown(InvItem.ItemEffectClass);
        if (TimeLeft > 0.0f)
        {
            UWarningSubsystem* WarningSys = GI->GetSubsystem<UWarningSubsystem>();
            if (WarningSys)
            {
                FText FormatPattern = FText::FromStringTable(
                    TEXT("/Game/character/ST_WarningMessages.ST_WarningMessages"),
                    TEXT("Err_Cooldown")
                );
                FNumberFormattingOptions NumFormat;
                NumFormat.MaximumFractionalDigits = 1;
                NumFormat.MinimumFractionalDigits = 1;
                FText FinalWarningText = FText::Format(FormatPattern, FText::AsNumber(TimeLeft, &NumFormat));
                WarningSys->ShowWarning(FinalWarningText);
            }
            return;
        }

        AMyCharacter* MyChar = Cast<AMyCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
        if (!MyChar) return;

        // 사용 불가 조건을 클라이언트에서 선검사합니다. (HP 최대치, 버프 중복 등)
        UItemEffectBase* ItemEffect = NewObject<UItemEffectBase>(this, InvItem.ItemEffectClass);
        if (!ItemEffect) return;

        if (!ItemEffect->CanUseItem(MyChar))
        {
            UWarningSubsystem* WarningSys = GI->GetSubsystem<UWarningSubsystem>();
            if (WarningSys)
            {
                WarningSys->ShowWarning(ItemEffect->UsageFailMessage);
            }
            return;
        }

        // 쿨다운을 클라이언트에서 즉시 등록해 연속 사용을 방지합니다.
        ApplyCooldown(InvItem.ItemEffectClass, ItemEffect->CooldownTime);

        // 실제 효과 적용과 수량 감소는 서버 RPC로 처리합니다.
        // 서버에서 처리 후 Content가 복제되면 OnRep_Inventory → OnInventoryUpdated → SyncQuickSlotsWithInventory 순서로 UI가 자동 갱신됩니다.
        MyChar->InventoryComp->ServerUseItem(TargetID, SlotIndex);
        return;
    }
}

// 아이템 사용 후 쿨다운 종료 시각(현재 시각 + CooldownTime)을 CooldownMap에 기록합니다.
void UQuickSlotSubsystem::ApplyCooldown(UClass* EffectClass, float CooldownTime)
{
    if (!EffectClass || CooldownTime <= 0.0f) return;

    double EndTime = FPlatformTime::Seconds() + CooldownTime;
    CooldownMap.Add(EffectClass, EndTime);
}

// 현재 시각과 CooldownMap의 종료 시각을 비교해 남은 쿨다운 시간(초)을 반환합니다.
// 쿨다운이 끝났거나 등록되지 않은 경우 0.0f를 반환합니다.
float UQuickSlotSubsystem::GetRemainingCooldown(UClass* EffectClass)
{
    if (!EffectClass || !CooldownMap.Contains(EffectClass)) return 0.0f;

    double EndTime    = CooldownMap[EffectClass];
    double CurrentTime = FPlatformTime::Seconds();

    float Remaining = (float)(EndTime - CurrentTime);
    return (Remaining > 0.0f) ? Remaining : 0.0f;
}

// 두 슬롯 인덱스의 데이터를 교환하고 UI 갱신을 알립니다 (드래그 앤 드롭 정렬).
void UQuickSlotSubsystem::SwapQuickSlots(int32 IndexA, int32 IndexB)
{
    if (!QuickSlotContent.IsValidIndex(IndexA) || !QuickSlotContent.IsValidIndex(IndexB)) return;
    if (IndexA == IndexB) return;

    QuickSlotContent.Swap(IndexA, IndexB);

    if (OnQuickSlotUpdated.IsBound())
    {
        OnQuickSlotUpdated.Broadcast();
    }
}

// 인벤토리 변경 시 호출되어 퀵슬롯의 수량을 동기화하고, 사라진 아이템 슬롯을 비웁니다.
void UQuickSlotSubsystem::SyncQuickSlotsWithInventory()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UInventorySubsystem* InvenSystem = GI->GetSubsystem<UInventorySubsystem>();
    if (!InvenSystem) return;

    bool bNeedUpdate = false;

    for (int32 i = 0; i < QuickSlotContent.Num(); i++)
    {
        if (!QuickSlotContent[i].ItemID.IsValid()) continue;

        bool bFoundInInven = false;

        for (const FItemData& InvenItem : InvenSystem->Content)
        {
            if (InvenItem.ItemID == QuickSlotContent[i].ItemID)
            {
                // 인벤토리 수량이 바뀐 경우(소모·이동 등) 퀵슬롯 캐시도 갱신합니다.
                if (QuickSlotContent[i].Quantity != InvenItem.Quantity)
                {
                    QuickSlotContent[i].Quantity = InvenItem.Quantity;
                    bNeedUpdate = true;
                }
                bFoundInInven = true;
                break;
            }
        }

        if (!bFoundInInven)
        {
            // 인벤토리에 더 이상 없는 아이템(완전 소모·드롭 등)은 슬롯을 비웁니다.
            QuickSlotContent[i] = FItemData();
            bNeedUpdate = true;
        }
    }

    if (bNeedUpdate && OnQuickSlotUpdated.IsBound())
    {
        OnQuickSlotUpdated.Broadcast();
    }
}
