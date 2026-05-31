#include "Skill/BuffListWidget.h"
#include "Components/HorizontalBox.h"
#include "Skill/BuffIconWidget.h"
#include "Skill/SkillComponent.h"
#include "MyCharacter.h" 
#include "Skill/SkillData.h"
#include "CombatComponent.h"
#include "ItemData.h"

// =========================================================================================
// BuffListWidget.cpp
//
// [파일 역할]
// 캐릭터에 걸린 버프 아이콘들을 화면에 가로로 나열하는 위젯입니다.
// SkillComponent의 OnBuffListUpdated 델리게이트를 구독하여 버프 변경 시 UI를 갱신합니다.
// =========================================================================================

void UBuffListWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 소유 캐릭터에서 SkillComponent와 CombatComponent를 찾아 캐싱하고 델리게이트를 바인딩합니다.
    if (AMyCharacter* MyChar = Cast<AMyCharacter>(GetOwningPlayerPawn()))
    {
        CachedSkillComp = MyChar->FindComponentByClass<USkillComponent>();

        if (CachedSkillComp)
        {
            CachedSkillComp->OnBuffListUpdated.AddDynamic(this, &UBuffListWidget::UpdateBuffList);
        }

        CachedCombatComp = MyChar->FindComponentByClass<UCombatComponent>();
        if (CachedCombatComp)
        {
            CachedCombatComp->OnCombatBuffUpdated.AddDynamic(this, &UBuffListWidget::UpdateBuffList);
        }

        UpdateBuffList();
    }

    GetWorld()->GetTimerManager().SetTimer(BuffUILogTimerHandle, this, &UBuffListWidget::LogBuffUIStatus, 1.0f, true);
}


void UBuffListWidget::NativeDestruct()
{
    Super::NativeDestruct();

    // 위젯이 닫힐 때 SkillComponent, CombatComponent 델리게이트를 해제합니다.
    // 두 컴포넌트는 플레이어에 붙어 계속 살아있으므로 명시적 해제가 필요합니다.
    if (CachedSkillComp)
    {
        CachedSkillComp->OnBuffListUpdated.RemoveDynamic(this, &UBuffListWidget::UpdateBuffList);
    }
    if (CachedCombatComp)
    {
        CachedCombatComp->OnCombatBuffUpdated.RemoveDynamic(this, &UBuffListWidget::UpdateBuffList);
    }

    GetWorld()->GetTimerManager().ClearTimer(BuffUILogTimerHandle);
}

void UBuffListWidget::UpdateBuffList()
{
    if (!CachedSkillComp || !BuffBox || !BuffIconClass) return;

    BuffBox->ClearChildren();

    // ========================================================
    // 1. 스킬 버프 아이콘 표시 (스킬 컴포넌트 버프 목록)
    // ========================================================
    for (const FActiveBuff& Buff : CachedSkillComp->ActiveBuffs)
    {
        UBuffIconWidget* IconWidget = CreateWidget<UBuffIconWidget>(GetWorld(), BuffIconClass);

        if (IconWidget)
        {
            UTexture2D* FoundIcon = nullptr;

            // 스킬 데이터 테이블에서 해당 버프 ID의 아이콘 이미지를 찾습니다.
            if (SkillDataTable)
            {
                FSkillData* Data = SkillDataTable->FindRow<FSkillData>(Buff.BuffID, TEXT("BuffContext"));
                if (Data) FoundIcon = Data->Icon;
            }

            // 찾은 아이콘(없으면 nullptr)과 시간 정보를 버프 아이콘 위젯에 전달합니다.
            IconWidget->InitBuff(FoundIcon, Buff.MaxDuration, Buff.RemainingTime);

            BuffBox->AddChildToHorizontalBox(IconWidget);
        }
    }

    // ========================================================
    // 2. 아이템 버프 표시 (CombatComponent::ActiveBuffs 순회)
    // 복제된 배열을 직접 읽으므로 서버·클라이언트 모두 동일하게 표시됩니다.
    // ========================================================
    if (CachedCombatComp && ItemDataTable)
    {
        for (const FActiveBuffInfo& Buff : CachedCombatComp->ActiveBuffs)
        {
            if (!Buff.bIsItemBuff) continue;

            FItemData* FoundItemData = ItemDataTable->FindRow<FItemData>(Buff.BuffID, TEXT("ItemBuffContext"));
            if (!FoundItemData) continue;

            UBuffIconWidget* PotionWidget = CreateWidget<UBuffIconWidget>(GetWorld(), BuffIconClass);
            if (!PotionWidget) continue;

            float TimeLeft = FMath::Max(0.0f, CachedCombatComp->ItemBuffClientEndTime - GetWorld()->GetTimeSeconds());

            PotionWidget->InitBuff(FoundItemData->ItemIcon, Buff.MaxDuration, TimeLeft);
            BuffBox->AddChildToHorizontalBox(PotionWidget);
        }
    }
}

// 1초마다 호출되어 버프 UI에 표시되는 남은 시간을 로그로 출력합니다.
void UBuffListWidget::LogBuffUIStatus()
{
    if (CachedSkillComp)
    {
        for (const FActiveBuff& Buff : CachedSkillComp->ActiveBuffs)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[BUFF_UI] SkillBuff=%s | UI표시시간=%.1f초"),
                *Buff.BuffID.ToString(), Buff.RemainingTime);
        }
    }

    if (CachedCombatComp)
    {
        for (const FActiveBuffInfo& Buff : CachedCombatComp->ActiveBuffs)
        {
            if (!Buff.bIsItemBuff) continue;
            float UITimeLeft = FMath::Max(0.0f, CachedCombatComp->ItemBuffClientEndTime - GetWorld()->GetTimeSeconds());
            UE_LOG(LogTemp, Warning,
                TEXT("[BUFF_UI] ItemBuff=%s | UI표시시간=%.1f초"),
                *Buff.BuffID.ToString(), UITimeLeft);
        }
    }
}