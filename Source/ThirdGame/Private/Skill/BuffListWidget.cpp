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