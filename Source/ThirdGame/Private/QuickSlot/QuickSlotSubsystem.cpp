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
// 쿨다운 남아있으면 경고 메시지 표시, 사용 조건 미충족 시 실패 메시지 표시,
// 통과 시 효과 적용 → 쿨다운 등록 → 수량 감소 → UI 갱신 순서로 처리합니다.
void UQuickSlotSubsystem::UseQuickSlot(int32 SlotIndex)
{
    if (!QuickSlotContent.IsValidIndex(SlotIndex)) return;

    FGuid TargetID = QuickSlotContent[SlotIndex].ItemID;
    if (!TargetID.IsValid()) return;

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UInventorySubsystem* InvenSystem = GI->GetSubsystem<UInventorySubsystem>();
    if (!InvenSystem) return;

    bool bItemFound = false;

    // 인벤토리에서 GUID로 아이템을 찾아 쿨다운·조건·효과 처리를 진행합니다.
    for (FItemData& InvItem : InvenSystem->Content)
    {
        if (InvItem.ItemID != TargetID) continue;

        bItemFound = true;

        if (InvItem.Quantity <= 0) break;

        if (!InvItem.ItemEffectClass) break;

        // 쿨다운이 남아 있으면 남은 시간을 경고창으로 표시하고 사용을 막습니다.
        float TimeLeft = GetRemainingCooldown(InvItem.ItemEffectClass);
        if (TimeLeft > 0.0f)
        {
            UWarningSubsystem* WarningSys = GetGameInstance()->GetSubsystem<UWarningSubsystem>();
            if (WarningSys)
            {
                // StringTable의 {0} 플레이스홀더에 남은 시간(소수점 1자리)을 채워 표시합니다.
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

        UItemEffectBase* ItemEffect = NewObject<UItemEffectBase>(this, InvItem.ItemEffectClass);
        if (!ItemEffect) break;

        AMyCharacter* MyChar = Cast<AMyCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
        if (!MyChar) break;

        // HP가 최대치이거나 버프가 이미 활성 중인 경우 등 사용 불가 조건을 검사합니다.
        if (!ItemEffect->CanUseItem(MyChar))
        {
            UWarningSubsystem* WarningSys = GetGameInstance()->GetSubsystem<UWarningSubsystem>();
            if (WarningSys)
            {
                WarningSys->ShowWarning(ItemEffect->UsageFailMessage);
            }
            break;
        }

        // 효과 적용 → 버프 Niagara VFX 스폰 → 쿨다운 등록
        ItemEffect->ExecuteItemEffect(MyChar);

        if (InvItem.BuffEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                InvItem.BuffEffect, MyChar->GetMesh(), NAME_None,
                FVector(0.f, 0.f, -90.f), FRotator::ZeroRotator,
                EAttachLocation::KeepRelativeOffset, true);
        }

        ApplyCooldown(InvItem.ItemEffectClass, ItemEffect->CooldownTime);

        // 수량을 1 감소하고 퀵슬롯 캐시도 동기화합니다.
        InvItem.Quantity--;
        QuickSlotContent[SlotIndex] = InvItem;

        // 수량이 0이 되면 인벤토리 슬롯과 퀵슬롯 모두 빈 데이터로 초기화합니다.
        if (InvItem.Quantity <= 0)
        {
            InvItem = FItemData();
            QuickSlotContent[SlotIndex] = FItemData();
        }

        break;
    }

    // 아이템을 찾아 처리가 완료됐으므로 인벤토리·퀵슬롯 UI를 모두 갱신합니다.
    if (bItemFound)
    {
        if (InvenSystem->OnInventoryUpdated.IsBound()) InvenSystem->OnInventoryUpdated.Broadcast();
        if (OnQuickSlotUpdated.IsBound())              OnQuickSlotUpdated.Broadcast();
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
