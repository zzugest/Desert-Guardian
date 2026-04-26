// =========================================================================================
// ItemEffect_StatPotion.cpp
//
// [파일 역할]
// 포션 계열 아이템의 효과(HP 회복 / MP 회복 / 공격력 버프)를 실제로 적용하는 클래스입니다.
// ItemEffectBase의 ExecuteItemEffect / CanUseItem 두 BlueprintNativeEvent를 C++로 구현합니다.
// 수치는 bIsPercentage 플래그에 따라 퍼센트 또는 절댓값으로 계산하며,
// 공격력 버프는 스킬 버프와 독립된 아이템 버프 경로(bFromItem=true)로 처리합니다.
// =========================================================================================

#include "Item/ItemEffect_StatPotion.h"
#include "GameFramework/Pawn.h"
#include "CombatComponent.h"

// 포션을 사용할 때 HP·MP 회복 및 공격력 버프를 CombatComponent에 적용합니다.
void UItemEffect_StatPotion::ExecuteItemEffect_Implementation(APawn* User)
{
    if (!User) return;

    // User의 CombatComponent를 가져와 실제 스탯 변경을 위임합니다.
    UCombatComponent* CombatComp = Cast<UCombatComponent>(
        User->GetComponentByClass(UCombatComponent::StaticClass()));
    if (!CombatComp) return;

    // HP 회복: bIsPercentage가 true이면 MaxHP 대비 비율로, false이면 절댓값으로 회복합니다.
    if (HealAmount > 0.0f)
    {
        float FinalHeal = bIsPercentage ? (CombatComp->MaxHP * (HealAmount / 100.0f)) : HealAmount;
        CombatComp->Heal(FinalHeal);
    }

    // MP 회복: bIsPercentage가 true이면 MaxMP 대비 비율로, false이면 절댓값으로 회복합니다.
    if (ManaAmount > 0.0f)
    {
        float FinalMana = bIsPercentage ? (CombatComp->MaxMP * (ManaAmount / 100.0f)) : ManaAmount;
        CombatComp->RestoreMana(FinalMana);
    }

    // 공격력 버프: bFromItem=true로 전달해 스킬 버프와 독립적으로 중첩 적용합니다.
    if (AttackBoostAmount > 0.0f)
    {
        float FinalAtk = bIsPercentage
            ? (CombatComp->BaseAttackPower * (AttackBoostAmount / 100.0f))
            : AttackBoostAmount;
        CombatComp->ApplyAttackBuff(FinalAtk, BuffDuration, ItemRowName, true);
    }
}

// 포션을 사용할 수 있는지 판단합니다. HP·MP가 최대치 미만이거나 버프가 미적용 상태일 때만 허용합니다.
bool UItemEffect_StatPotion::CanUseItem_Implementation(APawn* User)
{
    if (!User) return false;

    UCombatComponent* CombatComp = Cast<UCombatComponent>(
        User->GetComponentByClass(UCombatComponent::StaticClass()));
    if (!CombatComp) return false;

    bool bCanUse = false;

    // HP 회복 포션: 현재 HP가 최대 HP보다 낮을 때만 사용 가능합니다.
    if (HealAmount > 0.0f && CombatComp->CurrentHP < CombatComp->MaxHP)
    {
        bCanUse = true;
    }

    // MP 회복 포션: 현재 MP가 최대 MP보다 낮을 때만 사용 가능합니다.
    if (ManaAmount > 0.0f && CombatComp->CurrentMP < CombatComp->MaxMP)
    {
        bCanUse = true;
    }

    // 공격력 버프 포션: 아이템 버프가 이미 활성 중이면 중복 사용을 막습니다.
    // 스킬 버프(bIsAttackBuffActive)와는 독립적으로 체크합니다.
    if (AttackBoostAmount > 0.0f && !CombatComp->bIsItemAttackBuffActive)
    {
        bCanUse = true;
    }

    return bCanUse;
}
