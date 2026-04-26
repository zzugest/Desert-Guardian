#include "Skill/BuffListWidget.h"
#include "Components/HorizontalBox.h"
#include "Skill/BuffIconWidget.h"
#include "Skill/SkillComponent.h"
#include "MyCharacter.h" 
#include "Skill/SkillData.h"
#include "CombatComponent.h"
#include "ItemData.h"

// BuffListWidget.cpp
// Purpose:
//   - ĳ���Ϳ� �ɸ� ��Ƽ�� �������� ȭ�鿡 ���������� �����ϴ� ����.
//   - SkillComponent�� OnBuffListUpdated ��������Ʈ�� �����Ͽ� ���� �� UI ����.
// Key behaviors:
//   - NativeConstruct: �÷��̾��� SkillComponent�� ã�� �����ϰ� �ʱ� ��� ����.
//   - UpdateBuffList: ���� �������� ����� ActiveBuffs�� ���� ������ ������ �����.
// Safety notes:
//   - CachedSkillComp, BuffBox, BuffIconClass ������ ��ȿ�� �˻� �ʿ�.

void UBuffListWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // ���⼭ MyChar��� ���� �����ϴ� {
    if (AMyCharacter* MyChar = Cast<AMyCharacter>(GetOwningPlayerPawn()))
    {
        CachedSkillComp = MyChar->FindComponentByClass<USkillComponent>();

        if (CachedSkillComp)
        {
            CachedSkillComp->OnBuffListUpdated.AddDynamic(this, &UBuffListWidget::UpdateBuffList);
        }

        //  �ݵ�� MyChar�� �� { } ���ʿ��� �� �ڵ尡 ����Ǿ�� �մϴ�!
        CachedCombatComp = MyChar->FindComponentByClass<UCombatComponent>();
        if (CachedCombatComp)
        {
            CachedCombatComp->OnCombatBuffUpdated.AddDynamic(this, &UBuffListWidget::UpdateBuffList);
        }

        UpdateBuffList();
    } // ���⼭ MyChar ���� �����ϴ� }
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
}

void UBuffListWidget::UpdateBuffList()
{
    if (!CachedSkillComp || !BuffBox || !BuffIconClass) return;

    BuffBox->ClearChildren();

    // ========================================================
    // 1. ��ų ������ �׸��� (���� ���� ����)
    // ========================================================
    for (const FActiveBuff& Buff : CachedSkillComp->ActiveBuffs)
    {
        UBuffIconWidget* IconWidget = CreateWidget<UBuffIconWidget>(GetWorld(), BuffIconClass);

        //  �ϴ� ������ ���������� ����������� ������ ȭ�鿡 ��� �غ� �մϴ�!
        if (IconWidget)
        {
            UTexture2D* FoundIcon = nullptr;

            // ������ ���̺��� ���������� ������� ���� �̹����� ã���ϴ�.
            if (SkillDataTable)
            {
                FSkillData* Data = SkillDataTable->FindRow<FSkillData>(Buff.BuffID, TEXT("BuffContext"));
                if (Data) FoundIcon = Data->Icon;
            }

            // ã�� �̹���(������ nullptr)�� �ð��� ������ �Ѱ��ݴϴ�.
            IconWidget->InitBuff(FoundIcon, Buff.MaxDuration, Buff.RemainingTime);

            // ȭ�鿡 ����!
            BuffBox->AddChildToHorizontalBox(IconWidget);
        }
    }

    // ========================================================
    // 2. 아이템 버프 표시 (bIsItemAttackBuffActive 전용 경로)
    // 스킬 버프와 독립된 플래그를 사용하므로 중첩 표시가 가능합니다.
    // ========================================================
    if (CachedCombatComp && CachedCombatComp->bIsItemAttackBuffActive
        && !CachedCombatComp->ActiveItemAttackBuffID.IsNone())
    {
        float TimeLeft = GetWorld()->GetTimerManager().GetTimerRemaining(CachedCombatComp->ItemAttackBuffTimerHandle);

        if (TimeLeft > 0.0f && ItemDataTable)
        {
            FItemData* FoundItemData = ItemDataTable->FindRow<FItemData>(
                CachedCombatComp->ActiveItemAttackBuffID, TEXT("ItemBuffContext"));

            if (FoundItemData)
            {
                UBuffIconWidget* PotionWidget = CreateWidget<UBuffIconWidget>(GetWorld(), BuffIconClass);
                if (PotionWidget)
                {
                    float MaxTime = GetWorld()->GetTimerManager().GetTimerRate(CachedCombatComp->ItemAttackBuffTimerHandle);
                    PotionWidget->InitBuff(FoundItemData->ItemIcon, MaxTime, TimeLeft);
                    BuffBox->AddChildToHorizontalBox(PotionWidget);
                }
            }
        }
    }
}