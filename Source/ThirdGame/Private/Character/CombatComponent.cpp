// =========================================================================================
// CombatComponent.cpp
//
// [파일 역할]
// 플레이어 캐릭터의 전투 전반을 담당하는 컴포넌트입니다.
//
// [주요 기능]
// 1. 스탯 관리     : HP·MP·SP 초기화 및 레벨 전환 시 StatSubsystem으로 저장·복원
// 2. 근접 공격     : 3단 콤보(지상) / 점프 공격(공중), AnimNotify 기반 무기 스윕 트레이스
// 3. 마법 공격     : 3단 마법 콤보, 타겟 위치 SphereTrace로 범위 피해 적용
// 4. 버프 시스템   : 스킬 버프·아이템 버프를 독립 경로로 관리 (TMap 타이머 핸들)
// 5. 피격·회복     : 피해 수신, 힐, 마나 회복, 공격력 증가 처리 및 HUD 갱신
// =========================================================================================

#include "CombatComponent.h"
#include "Components/PostProcessComponent.h"

#include "Net/UnrealNetwork.h"
#include "MyCharacter.h"
#include "MySword.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "PlayerHUDWidget.h"
#include "StatSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "TargetingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "WarningSubsystem.h"
#include "Skill/SkillSubsystem.h"
#include "Enemy/Enemy.h"

#define ECC_WeaponTrace ECC_GameTraceChannel1

// Tick을 활성화합니다. (마나 자동 회복, 타겟 추적 회전에 사용)
UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicated(true);
}

// 복제할 변수를 등록합니다.
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCombatComponent, CurrentHP);
    DOREPLIFETIME(UCombatComponent, CurrentMP);
    DOREPLIFETIME(UCombatComponent, CurrentSP);
    DOREPLIFETIME(UCombatComponent, BaseAttackPower);
    DOREPLIFETIME(UCombatComponent, ActiveBuffs);
}

// ActiveBuffs 복제 수신 시 호출 — 클라이언트 측 버프 플래그와 ID를 동기화하고 위젯을 갱신합니다.
void UCombatComponent::OnRep_ActiveBuffs()
{
    bool bPrevItemBuffActive = bIsItemAttackBuffActive;

    bIsAttackBuffActive     = false;
    bIsItemAttackBuffActive = false;
    ActiveAttackBuffID      = NAME_None;
    ActiveItemAttackBuffID  = NAME_None;

    for (const FActiveBuffInfo& Buff : ActiveBuffs)
    {
        if (Buff.bIsItemBuff)
        {
            bIsItemAttackBuffActive = true;
            ActiveItemAttackBuffID  = Buff.BuffID;
        }
        else
        {
            bIsAttackBuffActive = true;
            ActiveAttackBuffID  = Buff.BuffID;
        }
    }

    // 아이템 버프가 새로 생겼을 때만 클라이언트 로컬 카운트다운을 시작합니다.
    if (bIsItemAttackBuffActive && !bPrevItemBuffActive)
    {
        for (const FActiveBuffInfo& Buff : ActiveBuffs)
        {
            if (Buff.bIsItemBuff)
            {
                ItemBuffClientEndTime = GetWorld()->GetTimeSeconds() + Buff.MaxDuration;
                break;
            }
        }
    }

    if (OnCombatBuffUpdated.IsBound())
    {
        OnCombatBuffUpdated.Broadcast();
    }
}

// CurrentHP/MP/SP 복제 수신 시 호출 — 소유 클라이언트의 HUD를 갱신합니다.
void UCombatComponent::OnRep_Stats()
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    if (Owner->PlayerHUD)
    {
        Owner->PlayerHUD->UpdateState(CurrentHP, MaxHP, CurrentMP, MaxMP, CurrentSP, MaxSP);
    }

    // HP가 0 이하면 로컬에서 사망 태그를 설정하고 화면을 흑백으로 전환합니다.
    if (CurrentHP <= 0.f)
    {
        Owner->AddStateTag("State.Dead");
        if (Owner->IsLocallyControlled() && Owner->DeathPostProcess)
        {
            Owner->DeathPostProcess->BlendWeight = 1.f;
        }
    }
    // HP가 0 초과면 사망 태그를 제거하고 화면을 원래대로 복구합니다. (부활 시)
    else
    {
        Owner->RemoveStateTag("State.Dead");
        if (Owner->IsLocallyControlled() && Owner->DeathPostProcess)
        {
            Owner->DeathPostProcess->BlendWeight = 0.f;
        }
    }
}

// 소유자를 AMyCharacter로 캐스트해 반환하는 내부 헬퍼 함수입니다.
AMyCharacter* UCombatComponent::GetCharacterOwner()
{
    return Cast<AMyCharacter>(GetOwner());
}

// 컴포넌트 시작 시 StatSubsystem에서 스탯을 복원하거나 초기값을 저장하고, 캐싱을 수행합니다.
void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return;

    UStatSubsystem* StatSystem = GI->GetSubsystem<UStatSubsystem>();
    if (!StatSystem) return;

    // 저장된 스탯이 있으면 복원, 없으면(최초 실행) 현재 기본값을 저장합니다.
    if (StatSystem->SavedCurrentHP >= 0.0f)
    {
        CurrentHP = StatSystem->SavedCurrentHP;
        CurrentMP = StatSystem->SavedCurrentMP;
        MaxHP     = StatSystem->SavedMaxHP;
        MaxMP     = StatSystem->SavedMaxMP;
        CurrentSP = StatSystem->SavedCurrentSP;
        MaxSP     = StatSystem->SavedMaxSP;
    }
    else
    {
        StatSystem->SavedCurrentHP = CurrentHP;
        StatSystem->SavedCurrentMP = CurrentMP;
        StatSystem->SavedMaxHP     = MaxHP;
        StatSystem->SavedMaxMP     = MaxMP;
        StatSystem->SavedCurrentSP = CurrentSP;
        StatSystem->SavedMaxSP     = MaxSP;
    }

    if (CurrentSP <= 0.0f)
    {
        CurrentSP = MaxSP;
    }

    // TargetingComponent와 WeaponMesh를 한 번만 조회해 캐싱합니다.
    AMyCharacter* CharOwner = GetCharacterOwner();
    if (CharOwner)
    {
        CachedTargetingComp = CharOwner->FindComponentByClass<UTargetingComponent>();

        TArray<USkeletalMeshComponent*> SkeletalMeshes;
        CharOwner->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
        for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
        {
            if (MeshComp->GetName() == TEXT("WeaponMesh"))
            {
                CachedWeaponMesh = MeshComp;
                break;
            }
        }
    }

    GetWorld()->GetTimerManager().SetTimer(BuffLogTimerHandle, this, &UCombatComponent::LogBuffStatus, 1.0f, true);

    UE_LOG(LogTemp, Warning, TEXT("[LV_TRAVEL] CombatComp BeginPlay | Auth:%s | HP:%.0f/%.0f | MP:%.0f/%.0f | SP:%.0f/%.0f | AtkPow:%.1f | SkillBuff:%s | ItemBuff:%s"),
        (GetOwner() && GetOwner()->HasAuthority()) ? TEXT("SERVER") : TEXT("CLIENT"),
        CurrentHP, MaxHP, CurrentMP, MaxMP, CurrentSP, MaxSP, BaseAttackPower,
        bIsAttackBuffActive     ? TEXT("ON") : TEXT("OFF"),
        bIsItemAttackBuffActive ? TEXT("ON") : TEXT("OFF"));

    // 레벨 이동 전에 저장해둔 버프 아이템 상태를 복원합니다.
    USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
    if (SkillSys)
    {
        bool  bWasActive;
        FName SavedBuffID;
        float SavedAmount;
        float SavedRemainingTime;
        SkillSys->LoadAttackBuffData(bWasActive, SavedBuffID, SavedAmount, SavedRemainingTime);

        if (bWasActive && SavedRemainingTime > 0.0f)
        {
            ApplyAttackBuff(SavedAmount, SavedRemainingTime, SavedBuffID);
        }
    }
}

// 레벨 전환 직전에 현재 스탯과 활성 버프 정보를 서브시스템에 저장하고 타이머를 정리합니다.
void UCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 마나 리젠은 UpdateStatToSubsystem을 별도로 호출하지 않으므로 여기서 반드시 저장합니다.
    UpdateStatToSubsystem();

    if (bIsAttackBuffActive)
    {
        float RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(AttackBuffTimerHandle);
        if (RemainingTime > 0.0f)
        {
            if (UGameInstance* GI = GetWorld()->GetGameInstance())
            {
                if (USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>())
                {
                    SkillSys->SaveAttackBuffData(true, ActiveAttackBuffID, ActiveAttackBuffAmount, RemainingTime);
                }
            }
        }
    }

    // 레벨 전환·종료 시 타이머를 명시적으로 정리합니다.
    // 타이머가 살아있으면 GC 이후 콜백이 호출되어 크래시가 발생할 수 있습니다.
    GetWorld()->GetTimerManager().ClearTimer(AttackBuffTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(MagicComboTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(BuffLogTimerHandle);

    Super::EndPlay(EndPlayReason);
}

// 1초마다 호출되어 활성 버프의 실제 남은 시간을 로그로 출력합니다.
void UCombatComponent::LogBuffStatus()
{
    if (ActiveBuffs.IsEmpty()) return;

    float SkillTimerLeft = GetWorld()->GetTimerManager().GetTimerRemaining(AttackBuffTimerHandle);
    float ItemTimerLeft  = GetWorld()->GetTimerManager().GetTimerRemaining(ItemAttackBuffTimerHandle);

    for (const FActiveBuffInfo& Buff : ActiveBuffs)
    {
        float ClientTimeLeft = Buff.bIsItemBuff
            ? FMath::Max(0.0f, ItemBuffClientEndTime - GetWorld()->GetTimeSeconds())
            : FMath::Max(0.0f, SkillTimerLeft);
        float TimerLeft      = Buff.bIsItemBuff ? ItemTimerLeft : SkillTimerLeft;
        const TCHAR* Auth    = (GetOwner() && GetOwner()->HasAuthority()) ? TEXT("SERVER") : TEXT("CLIENT");
        const TCHAR* IsItem  = Buff.bIsItemBuff ? TEXT("true") : TEXT("false");

        UE_LOG(LogTemp, Warning,
            TEXT("[BUFF_REAL] Auth:%s | BuffID=%s | IsItem=%s | ClientLocal=%.1f s | TimerBased=%.1f s"),
            Auth, *Buff.BuffID.ToString(), IsItem, ClientTimeLeft, TimerLeft);
    }
}

// 현재 HP·MP·SP 수치를 StatSubsystem에 저장해 레벨 전환 후에도 유지되도록 합니다.
void UCombatComponent::UpdateStatToSubsystem()
{
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return;

    UStatSubsystem* StatSystem = GI->GetSubsystem<UStatSubsystem>();
    if (!StatSystem) return;

    StatSystem->SavedCurrentHP = CurrentHP;
    StatSystem->SavedCurrentMP = CurrentMP;
    StatSystem->SavedMaxHP     = MaxHP;
    StatSystem->SavedMaxMP     = MaxMP;
    StatSystem->SavedCurrentSP = CurrentSP;
    StatSystem->SavedMaxSP     = MaxSP;
}

// 매 프레임 마나 자동 회복, 스킬 쿨다운 감소, 공격 중 타겟 방향 자동 회전을 처리합니다.
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AMyCharacter* Owner = GetCharacterOwner();

    // 마나 자동 회복은 서버에서만 처리합니다. 변경된 값은 OnRep_Stats를 통해 클라이언트 HUD에 자동 반영됩니다.
    if (Owner && Owner->HasAuthority() && CurrentMP < MaxMP)
    {
        CurrentMP += (ManaRegenRate * DeltaTime);
        if (CurrentMP > MaxMP) CurrentMP = MaxMP;
    }

    if (CurrentCooldown_Q > 0.0f)
    {
        CurrentCooldown_Q -= DeltaTime;
        if (CurrentCooldown_Q <= 0.0f) CurrentCooldown_Q = 0.0f;
    }

    // 공격 중 타겟 방향 회전은 각 콤보 단계 시작 시 PlayComboMontageOnly()의 스냅으로 처리합니다.
}

// 공격 입력을 받아 현재 상태(공중/지상/콤보 중)에 따라 적절한 공격 루틴으로 분기합니다.
void UCombatComponent::Attack()
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    if (Owner->HasStateTag("State.CC.Stun") || Owner->HasStateTag("State.Action.Rolling")) return;
    if (Owner->HasStateTag("State.Action.MagicCasting")) return;

    // 공격 중 입력이 들어왔다면 콤보 입력 플래그만 세우고 대기합니다.
    if (Owner->HasStateTag("State.Action.Attacking"))
    {
        bIsComboInputOn = true;
        return;
    }

    if (Owner->HasStateTag("State.Movement.InAir"))
    {
        PlayJumpAttackAnim();
    }
    else
    {
        PlayComboAnim();
    }
}

// 지상에서의 콤보 공격 애니메이션을 현재 콤보 단계에 맞는 섹션으로 재생합니다.
// 콤보 카운터·상태태그 관리(서버 로직)를 처리한 뒤 PlayComboMontageOnly()를 호출합니다.
void UCombatComponent::PlayComboAnim()
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    // 공격 중에는 SetActorRotation으로 타겟을 향해 직접 회전하므로
    // bOrientRotationToMovement와 bUseControllerRotationYaw를 모두 끕니다.
    Owner->bUseControllerRotationYaw = false;
    if (Owner->GetCharacterMovement())
    {
        Owner->GetCharacterMovement()->bOrientRotationToMovement = false;
    }

    CurrentCombo = FMath::Clamp(CurrentCombo + 1, 1, MaxCombo);
    Owner->AddStateTag("State.Action.Attacking");
    bIsComboInputOn = false;

    // 공격 시작 시점의 타겟을 캐싱합니다.
    // TargetingComponent는 0.1초 간격으로 갱신되므로 타이밍에 따라 CurrentTarget이 null이 될 수 있습니다.
    // 공격 내내 일관된 타겟을 사용하기 위해 이 시점에 한 번만 저장합니다.
    CombatTargetActor = (CachedTargetingComp) ? CachedTargetingComp->CurrentTarget : nullptr;

    // 실제 몽타주 재생은 PlayComboMontageOnly()가 담당합니다.
    // (Multicast RPC를 통해 모든 클라이언트에서 호출됩니다.)
    PlayComboMontageOnly(CurrentCombo);
}

// 콤보 카운터·상태태그 처리 없이 몽타주 재생만 담당합니다.
// Multicast RPC(MulticastPlayComboMontage)에서 모든 클라이언트에 일괄 호출됩니다.
void UCombatComponent::PlayComboMontageOnly(int32 ComboIndex)
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    // 클라이언트도 서버와 동일한 회전 플래그 상태를 유지합니다.
    // SetActorRotation(TickComponent)이 충돌 없이 타겟 방향으로 회전할 수 있도록 합니다.
    Owner->bUseControllerRotationYaw = false;
    if (Owner->GetCharacterMovement())
    {
        Owner->GetCharacterMovement()->bOrientRotationToMovement = false;
    }

    // 타겟이 있으면 몽타주 재생 전 즉시 해당 방향을 바라봅니다.
    // 루트 모션이 처음부터 올바른 방향으로 출발하도록 합니다.
    if (Owner->IsLocallyControlled() && IsValid(CombatTargetActor))
    {
        FVector ToTarget = (CombatTargetActor->GetActorLocation() - Owner->GetActorLocation());
        ToTarget.Z = 0.f;
        if (!ToTarget.IsNearlyZero())
        {
            Owner->SetActorRotation(ToTarget.Rotation());
        }
    }

    UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
    if (!AnimInstance || !ComboActionMontage) return;

    // 루트 모션을 몽타주에서만 받아 공격 이동 거리를 정밀하게 제어합니다.
    AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);

    // Montage_IsPlaying은 블렌드 아웃 중일 때 false를 반환합니다.
    // Montage_IsActive는 블렌드 아웃 중에도 true를 반환하므로
    // 콤보 타이밍 경계에서 몽타주가 처음부터 재시작되는 문제를 방지합니다.
    if (!AnimInstance->Montage_IsActive(ComboActionMontage))
    {
        AnimInstance->Montage_Play(ComboActionMontage);

        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UCombatComponent::OnAttackMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, ComboActionMontage);
    }

    FString SectionName = FString::Printf(TEXT("Attack%d"), ComboIndex);
    AnimInstance->Montage_JumpToSection(FName(*SectionName), ComboActionMontage);
}

// AnimNotify가 호출될 때 콤보 입력이 있으면 다음 콤보 단계를 바로 재생합니다.
void UCombatComponent::ProcessComboCommand()
{
    if (!bIsComboInputOn) return;

    bIsComboInputOn = false;

    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    if (Owner->HasStateTag("State.Movement.InAir"))
    {
        PlayJumpLoopAnim();
    }
    else
    {
        PlayComboAnim();
    }
}

// 공격 몽타주가 끝나면 공격 상태 태그를 제거하고 카메라·이동 방향 설정을 원래대로 복구합니다.
void UCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (Owner)
    {
        Owner->RemoveStateTag("State.Action.Attacking");
        Owner->RemoveStateTag(FName("State.Action.JumpLanding"));
        Owner->RemoveStateTag(FName("State.Action.JumpLoop"));

        UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
        }

        // 전투 자세면 컨트롤러 Yaw 유지, 아니면 이동 방향 자동 회전으로 복귀합니다.
        if (Owner->HasStateTag("State.Stance.Combat"))
        {
            Owner->bUseControllerRotationYaw = true;
            if (Owner->GetCharacterMovement()) Owner->GetCharacterMovement()->bOrientRotationToMovement = false;
        }
        else
        {
            Owner->bUseControllerRotationYaw = false;
            if (Owner->GetCharacterMovement()) Owner->GetCharacterMovement()->bOrientRotationToMovement = true;
        }
    }

    bIsComboInputOn   = false;
    CurrentCombo      = 0;
    CombatTargetActor = nullptr;
}

// 공중에서 공격 키 입력 시 점프 공격 몽타주의 'Start' 섹션을 재생합니다.
void UCombatComponent::PlayJumpAttackAnim()
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner || !JumpAttackMontage) return;

    Owner->bUseControllerRotationYaw = false;
    if (Owner->GetCharacterMovement())
    {
        Owner->GetCharacterMovement()->bOrientRotationToMovement = true;
    }

    Owner->AddStateTag(FName("State.Action.Attacking"));

    UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    // 공중이므로 루트 모션을 무시하고 물리 이동을 그대로 유지합니다.
    AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);

    AnimInstance->Montage_Play(JumpAttackMontage);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &UCombatComponent::OnAttackMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, JumpAttackMontage);
}

// 점프 공격 'Start' 섹션이 끝나면 'Loop' 섹션으로 이동해 착지 대기 상태로 전환합니다.
void UCombatComponent::PlayJumpLoopAnim()
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner || !JumpAttackMontage) return;

    UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
    if (!AnimInstance || !AnimInstance->Montage_IsPlaying(JumpAttackMontage)) return;

    AnimInstance->Montage_JumpToSection(FName("Loop"), JumpAttackMontage);
}

// 착지 시 현재 재생 중인 점프 공격 섹션에 따라 'End' 섹션으로 전환하거나 몽타주를 강제 종료합니다.
void UCombatComponent::PlayJumpLandingAnim()
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner || !JumpAttackMontage) return;

    UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
    if (!AnimInstance || !AnimInstance->Montage_IsPlaying(JumpAttackMontage)) return;

    FName CurrentSection = AnimInstance->Montage_GetCurrentSection(JumpAttackMontage);

    if (CurrentSection == FName("Loop"))
    {
        Owner->AddStateTag(FName("State.Action.JumpLanding"));
        AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
        AnimInstance->Montage_JumpToSection(FName("End"), JumpAttackMontage);
    }
    else if (CurrentSection == FName("Start"))
    {
        // Start 구간에 너무 빨리 착지하면 즉시 몽타주를 중단하고 상태를 초기화합니다.
        AnimInstance->Montage_Stop(0.1f, JumpAttackMontage);
        EndWeaponTrace();

        bIsComboInputOn = false;
        Owner->RemoveStateTag(FName("State.Action.Attacking"));
        Owner->RemoveStateTag(FName("State.Action.JumpLoop"));
        Owner->RemoveStateTag(FName("State.Action.JumpLanding"));
    }
}

// AnimNotifyState에서 무기 충돌을 켜거나 끄는 플래그를 설정합니다. (현재 트레이스 방식으로 대체)
void UCombatComponent::SetWeaponCollision(bool bEnable)
{
    bIsWeaponEnabled = bEnable;
}

// 마나를 소모하고 Q 스킬 몽타주를 재생합니다. 쿨다운 중이거나 마나가 부족하면 실행하지 않습니다.
void UCombatComponent::SkillQ()
{
    if (CurrentCooldown_Q > 0.0f) return;

    float SkillCost = 20.0f;
    if (CurrentMP < SkillCost) return;

    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner || !SkillMontage) return;

    CurrentMP -= SkillCost;
    CurrentCooldown_Q = MaxCooldown_Q;

    UpdateStatToSubsystem();

    // CurrentMP가 Replicated이므로 서버에서 변경하면 OnRep_Stats가 클라이언트 HUD를 자동 갱신합니다.

    UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->Montage_Play(SkillMontage);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &UCombatComponent::OnAttackMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, SkillMontage);
}

// 외부에서 체력 회복 요청을 받아 HP를 최대치를 넘지 않게 증가시키고 HUD를 갱신합니다.
void UCombatComponent::Heal(float Amount)
{
    CurrentHP = FMath::Clamp(CurrentHP + Amount, 0.0f, MaxHP);

    // CurrentHP가 Replicated이므로 서버에서 변경하면 OnRep_Stats가 클라이언트 HUD를 자동 갱신합니다.

    UpdateStatToSubsystem();
}

// 외부에서 마나 회복 요청을 받아 MP를 최대치를 넘지 않게 증가시키고 HUD를 갱신합니다.
void UCombatComponent::RestoreMana(float Amount)
{
    CurrentMP = FMath::Clamp(CurrentMP + Amount, 0.0f, MaxMP);

    // CurrentMP가 Replicated이므로 서버에서 변경하면 OnRep_Stats가 클라이언트 HUD를 자동 갱신합니다.

    UpdateStatToSubsystem();
}

// 기본 공격력을 지정한 양만큼 증가시킵니다. 버프 적용·해제 시 양수·음수로 호출합니다.
void UCombatComponent::IncreaseAttackPower(float Amount)
{
    BaseAttackPower += Amount;
    UpdateStatToSubsystem();
}

// 무기 트레이스 시작 시 중복 타격 방지를 위한 히트 목록을 초기화하고 공격 데이터를 캐싱합니다.
void UCombatComponent::StartWeaponTrace(FName HitAttackID)
{
    CurrentAttackID  = HitAttackID;
    AlreadyHitActors.Empty();

    // 공격 시작 시 한 번만 DataTable을 조회해 캐싱합니다.
    CachedAttackData = AttackDataTable ? AttackDataTable->FindRow<FAttackData>(HitAttackID, TEXT("")) : nullptr;
}

// 무기 트레이스를 종료하고 관련 데이터를 초기화합니다.
void UCombatComponent::EndWeaponTrace()
{
    CurrentAttackID  = NAME_None;
    AlreadyHitActors.Empty();
    CachedAttackData = nullptr;
}

// AnimNotifyState의 Tick에서 호출되며, 무기 소켓 위치로 SweepTrace해 적에게 피해를 입힙니다.
// TSet(AlreadyHitActors)으로 한 공격당 동일 적에게 중복 피해가 가지 않도록 방지합니다.
void UCombatComponent::WeaponTraceTick()
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner || CurrentAttackID.IsNone() || !AttackDataTable) return;

    if (!CachedWeaponMesh) return;

    // 트레이스는 이 캐릭터를 직접 조작하는 머신에서만 실행합니다.
    // 서버는 다른 클라이언트의 캐릭터에 대해 독립적으로 트레이스하지 않습니다.
    if (!Owner->IsLocallyControlled()) return;

    FVector BaseLoc = CachedWeaponMesh->GetSocketLocation(FName("Base"));
    FVector TipLoc  = CachedWeaponMesh->GetSocketLocation(FName("Tip"));

    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);
    QueryParams.AddIgnoredActor(CurrentWeapon);

    float BladeThickness = 10.0f;
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        BaseLoc, TipLoc, FQuat::Identity,
        ECC_WeaponTrace,
        FCollisionShape::MakeSphere(BladeThickness),
        QueryParams
    );

    if (!bHit) return;
    if (!CachedAttackData) return;

    float FinalDamage = BaseAttackPower * CachedAttackData->DamageMultiplier;

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();

        // 이미 이번 공격에서 피해를 준 대상이면 건너뜁니다.
        if (!HitActor || AlreadyHitActors.Contains(HitActor)) continue;

        AlreadyHitActors.Add(HitActor);

        AEnemy* HitEnemy = Cast<AEnemy>(HitActor);
        if (HitEnemy && HitEnemy->bIsDead) continue;

        if (Owner->HasAuthority())
        {
            // 리슨 서버 본인: 트레이스 결과를 직접 서버에 적용합니다.
            UGameplayStatics::ApplyDamage(HitActor, FinalDamage, Owner->GetController(), Owner, UDamageType::StaticClass());
        }
        else
        {
            // 클라이언트: 맞은 액터와 위치를 서버에 전달해 데미지 처리를 위임합니다.
            // 서버에서 AttackID로 데미지를 재계산하므로 클라이언트 조작 불가합니다.
            Owner->Server_ReportHitEnemy(HitActor, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), CurrentAttackID);
        }

        // 히트 위치에 이펙트 스폰 (Niagara 우선, 없으면 Cascade)
        if (HitEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(), HitEffect,
                Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
        }
        else if (HitEffectCascade)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(), HitEffectCascade,
                Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
        }

        // 데미지 텍스트는 공격한 본인 화면에서만 표시합니다.
        if (Owner->IsLocallyControlled())
        {
            Owner->OnSpawnDamageText(Hit.ImpactPoint, FinalDamage);
        }
        Owner->EnterCombatStance();
    }
}

// 특정 공격 타입 히트를 실행하기 위해 예약된 함수입니다. (현재 미구현)
void UCombatComponent::ExecuteAttackHit(FName HitAttackID)
{
}

// 클라이언트가 보고한 히트를 서버에서 처리합니다.
// AttackID로 데미지를 서버에서 직접 재계산해 클라이언트 조작을 방지합니다.
void UCombatComponent::ServerApplyHitDamage(AActor* HitEnemy, FVector HitLocation, FRotator HitNormal, FName AttackID)
{
    if (!HitEnemy || !AttackDataTable) return;

    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    FAttackData* AttackData = AttackDataTable->FindRow<FAttackData>(AttackID, TEXT("ClientHitReport"));
    if (!AttackData) return;

    float FinalDamage = BaseAttackPower * AttackData->DamageMultiplier;

    UGameplayStatics::ApplyDamage(HitEnemy, FinalDamage, Owner->GetController(), Owner, UDamageType::StaticClass());
    SpawnHitEffectAt(HitLocation, HitNormal);
}

// 클라이언트가 보고한 마법 AoE 히트를 서버에서 처리합니다.
// MagicAttackID로 데미지를 서버에서 직접 재계산해 클라이언트 조작을 방지합니다.
void UCombatComponent::ServerApplyMagicDamage(const TArray<AActor*>& HitEnemies, FName MagicAttackID)
{
    if (!AttackDataTable) return;

    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    FAttackData* Data = AttackDataTable->FindRow<FAttackData>(MagicAttackID, TEXT("MagicHitReport"));
    if (!Data) return;

    float FinalDamage = BaseAttackPower * Data->DamageMultiplier;

    for (AActor* Enemy : HitEnemies)
    {
        if (!Enemy) continue;
        UGameplayStatics::ApplyDamage(Enemy, FinalDamage, Owner->GetController(), Owner, UDamageType::StaticClass());
    }
}

// 피해를 받아 HP를 감소시킵니다. HP가 0 이하가 되면 사망 처리를 추가할 위치입니다.
void UCombatComponent::ReceiveDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f || CurrentHP <= 0.0f) return;

    CurrentHP -= DamageAmount;
    CurrentHP  = FMath::Max(0.0f, CurrentHP);

    if (CurrentHP <= 0.0f)
    {
        AMyCharacter* Owner = GetCharacterOwner();
        if (Owner)
        {
            Owner->Die();
        }
    }
}

// 우클릭 입력 시 타겟이 존재하고 마나가 충분하면 마법 콤보 몽타주를 재생합니다.
void UCombatComponent::RightClickMagicAttack()
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner || Owner->HasStateTag("State.CC.Stun") || Owner->HasStateTag("State.Action.Rolling") || Owner->HasStateTag("State.Action.Attacking")) return;
    if (Owner->HasStateTag("State.Action.MagicCasting")) return;

    // 타겟이 없으면 마법을 시전하지 않습니다.
    if (!Owner->TargetingComp || !Owner->TargetingComp->CurrentTarget)
    {
        return;
    }

    UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
    if (!AnimInstance || !MagicComboMontage) return;
    if (AnimInstance->Montage_IsPlaying(MagicComboMontage)) return;

    float MagicCost = 15.0f;

    if (CurrentMP < MagicCost)
    {
        // 마나 부족 시 WarningSubsystem을 통해 경고 메시지를 화면에 표시합니다.
        if (Owner->GetGameInstance())
        {
            UWarningSubsystem* WarningSys = Owner->GetGameInstance()->GetSubsystem<UWarningSubsystem>();
            if (WarningSys)
            {
                FText WarningText = FText::FromStringTable(
                    TEXT("/Game/character/ST_WarningMessages.ST_WarningMessages"),
                    TEXT("Err_NotEnoughMP"));
                WarningSys->ShowWarning(WarningText);
            }
        }
        return;
    }

    CurrentMP -= MagicCost;
    UpdateStatToSubsystem();

    // CurrentMP가 Replicated이므로 서버에서 변경하면 OnRep_Stats가 클라이언트 HUD를 자동 갱신합니다.

    CurrentMagicCombo = FMath::Clamp(CurrentMagicCombo + 1, 1, MaxMagicCombo);
    GetWorld()->GetTimerManager().ClearTimer(MagicComboTimerHandle);

    // MagicCasting 태그를 부여해 이동 함수에서 공격 중 이동을 차단합니다.
    Owner->AddStateTag("State.Action.MagicCasting");
    Owner->EnterCombatStance();

    // 실제 몽타주 재생은 PlayMagicMontageOnly()가 담당합니다.
    // (Multicast RPC를 통해 모든 클라이언트에서 호출됩니다.)
    PlayMagicMontageOnly(CurrentMagicCombo);
}

// 마법 콤보 로직 처리 없이 몽타주 재생만 담당합니다.
// Multicast RPC(MulticastPlayMagicMontage)에서 모든 클라이언트에 일괄 호출됩니다.
// TargetLocation이 유효하면 AnimNotify(SpawnMagicVFX)가 실행되기 전에 CachedMagicLocation을 미리 설정합니다.
void UCombatComponent::PlayMagicMontageOnly(int32 ComboIndex, FVector TargetLocation)
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    // AnimNotify(SpawnMagicVFX, ApplyMagicDamage)가 CurrentMagicCombo를 참조하므로
    // 몽타주 재생 전에 반드시 동기화합니다. (서버에서만 RightClickMagicAttack()이 설정하기 때문)
    CurrentMagicCombo = ComboIndex;

    // 서버로부터 전달받은 타겟 위치를 몽타주 재생 전에 미리 캐싱합니다.
    // AnimNotify(SpawnMagicVFX)가 실행될 때 CurrentTarget이 없어도 올바른 위치에 VFX를 스폰합니다.
    if (!TargetLocation.IsZero())
    {
        CachedMagicLocation = TargetLocation;
    }

    UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
    if (!AnimInstance || !MagicComboMontage) return;

    AnimInstance->Montage_Play(MagicComboMontage);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &UCombatComponent::OnMagicMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, MagicComboMontage);

    FString SectionName = FString::Printf(TEXT("Attack%d"), ComboIndex);
    AnimInstance->Montage_JumpToSection(FName(*SectionName), MagicComboMontage);
}

// 마법 몽타주가 끝나면 MagicCasting 태그를 제거하고 콤보 리셋 타이머를 설정합니다.
void UCombatComponent::OnMagicMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    Owner->RemoveStateTag("State.Action.MagicCasting");

    if (!bInterrupted && CurrentMagicCombo < MaxMagicCombo)
    {
        // 2초 안에 다시 마법을 시전하면 콤보가 이어집니다.
        GetWorld()->GetTimerManager().SetTimer(MagicComboTimerHandle, this, &UCombatComponent::ResetMagicCombo, 2.0f, false);
    }
    else
    {
        ResetMagicCombo();
    }
}

// 마법 콤보 카운터를 0으로 초기화하고 콤보 유지 타이머를 해제합니다.
void UCombatComponent::ResetMagicCombo()
{
    CurrentMagicCombo = 0;
    GetWorld()->GetTimerManager().ClearTimer(MagicComboTimerHandle);
}

// AnimNotify에서 호출되며, 타겟 위치에 마법 이펙트(Niagara)를 스폰합니다.
// CurrentTarget이 있으면 최신 위치를 갱신하고, 없으면 PlayMagicMontageOnly에서 미리 설정한 CachedMagicLocation을 사용합니다.
void UCombatComponent::SpawnMagicVFX()
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner) return;

    // CurrentTarget이 있으면 최신 위치로 갱신합니다. (로컬 조종 캐릭터 또는 서버 측)
    // 없으면 PlayMagicMontageOnly에서 Multicast를 통해 미리 전달받은 CachedMagicLocation을 그대로 사용합니다.
    if (Owner->TargetingComp && Owner->TargetingComp->CurrentTarget)
    {
        CachedMagicLocation = Owner->TargetingComp->CurrentTarget->GetActorLocation();
    }

    // 유효한 위치가 없으면 VFX를 스폰하지 않습니다.
    if (CachedMagicLocation.IsZero()) return;

    int32 EffectIndex = CurrentMagicCombo - 1;
    if (MagicExplosionVFXs.IsValidIndex(EffectIndex) && MagicExplosionVFXs[EffectIndex] != nullptr)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            MagicExplosionVFXs[EffectIndex],
            CachedMagicLocation,
            FRotator::ZeroRotator,
            FVector(1.0f),
            true, true,
            ENCPoolMethod::AutoRelease);
    }
}

// AnimNotify에서 호출되며, 캐싱된 타겟 위치 주변을 SphereTrace해 범위 내 적에게 마법 피해를 입힙니다.
void UCombatComponent::ApplyMagicDamage()
{
    AMyCharacter* Owner = GetCharacterOwner();
    if (!Owner || !AttackDataTable) return;

    // 트레이스는 이 캐릭터를 직접 조작하는 머신에서만 실행합니다.
    if (!Owner->IsLocallyControlled()) return;

    FString RowNameStr    = FString::Printf(TEXT("Magic%d"), CurrentMagicCombo);
    FName   MagicAttackID = FName(*RowNameStr);
    FAttackData* Data     = AttackDataTable->FindRow<FAttackData>(MagicAttackID, TEXT("MagicExplosion"));
    if (!Data) return;

    float FinalDamage  = BaseAttackPower * Data->DamageMultiplier;
    float DamageRadius = 300.0f;

    TArray<AActor*>    IgnoredActors;
    TArray<FHitResult> HitResults;
    IgnoredActors.Add(Owner);

    bool bHit = UKismetSystemLibrary::SphereTraceMulti(
        GetWorld(),
        CachedMagicLocation, CachedMagicLocation, DamageRadius,
        UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),
        false, IgnoredActors,
        EDrawDebugTrace::None,
        HitResults, true,
        FLinearColor::Red, FLinearColor::Green, 2.0f);

    if (bHit)
    {
        TArray<AActor*> DamagedActors;
        TArray<AActor*> HitEnemiesForServer;

        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (!HitActor || DamagedActors.Contains(HitActor)) continue;

            APawn* HitPawn = Cast<APawn>(HitActor);
            if (!HitPawn) continue;

            AEnemy* HitEnemy = Cast<AEnemy>(HitPawn);
            if (HitEnemy && HitEnemy->bIsDead) continue;

            if (Owner->HasAuthority())
            {
                // 리슨 서버 본인: 직접 데미지 적용
                UGameplayStatics::ApplyDamage(HitPawn, FinalDamage, Owner->GetController(), Owner, UDamageType::StaticClass());
            }
            else
            {
                // 클라이언트: 서버에 보고할 목록에 추가
                HitEnemiesForServer.Add(HitActor);
            }

            // 데미지 텍스트는 공격한 본인 화면에서만 표시합니다.
            Owner->OnSpawnDamageText(Hit.ImpactPoint, FinalDamage);

            DamagedActors.Add(HitActor);
        }

        // 클라이언트: 맞은 적 목록을 서버에 한 번에 보고합니다.
        if (!Owner->HasAuthority() && HitEnemiesForServer.Num() > 0)
        {
            Owner->Server_ReportMagicHit(HitEnemiesForServer, MagicAttackID);
        }
    }

    Owner->EnterCombatStance();
}

// 지정 위치에 히트 이펙트를 스폰합니다.
// ServerReportWeaponHit RPC를 통해 클라이언트가 보고한 히트 위치에 서버에서 직접 스폰합니다.
void UCombatComponent::SpawnHitEffectAt(FVector HitLocation, FRotator HitNormal)
{
    if (HitEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), HitEffect, HitLocation, HitNormal);
    }
    else if (HitEffectCascade)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), HitEffectCascade, HitLocation, HitNormal);
    }
}

// 스킬 또는 아이템 버프를 적용합니다. bFromItem 플래그로 두 경로를 완전히 분리해 중첩을 허용합니다.
void UCombatComponent::ApplyAttackBuff(float Amount, float Duration, FName BuffID, bool bFromItem)
{
    if (bFromItem)
    {
        // 아이템 버프 경로: 스킬 버프와 독립적으로 관리
        if (bIsItemAttackBuffActive) return;

        bIsItemAttackBuffActive    = true;
        ActiveItemAttackBuffAmount = Amount;
        ActiveItemAttackBuffID     = BuffID;

        IncreaseAttackPower(Amount);
        GetWorld()->GetTimerManager().SetTimer(ItemAttackBuffTimerHandle, this, &UCombatComponent::RemoveItemAttackBuff, Duration, false);
        ItemBuffClientEndTime = GetWorld()->GetTimeSeconds() + Duration;
    }
    else
    {
        // 스킬 버프 경로
        if (bIsAttackBuffActive) return;

        bIsAttackBuffActive    = true;
        ActiveAttackBuffAmount = Amount;
        ActiveAttackBuffID     = BuffID;

        IncreaseAttackPower(Amount);
        GetWorld()->GetTimerManager().SetTimer(AttackBuffTimerHandle, this, &UCombatComponent::RemoveAttackBuff, Duration, false);
    }

    FActiveBuffInfo NewBuff;
    NewBuff.BuffID      = BuffID;
    NewBuff.Amount      = Amount;
    NewBuff.MaxDuration = Duration;
    NewBuff.bIsItemBuff = bFromItem;
    ActiveBuffs.Add(NewBuff);

    if (OnCombatBuffUpdated.IsBound())
    {
        OnCombatBuffUpdated.Broadcast();
    }
}

// 스킬 버프 타이머가 만료되면 공격력을 원래대로 되돌리고 버프 상태를 초기화합니다.
void UCombatComponent::RemoveAttackBuff()
{
    bIsAttackBuffActive    = false;
    IncreaseAttackPower(-ActiveAttackBuffAmount);
    ActiveAttackBuffAmount = 0.0f;

    ActiveBuffs.RemoveAll([this](const FActiveBuffInfo& Buff)
    {
        return !Buff.bIsItemBuff && Buff.BuffID == ActiveAttackBuffID;
    });

    ActiveAttackBuffID = NAME_None;

    if (OnCombatBuffUpdated.IsBound())
    {
        OnCombatBuffUpdated.Broadcast();
    }
}

// 아이템 버프 타이머가 만료되면 공격력을 원래대로 되돌리고 아이템 버프 상태를 초기화합니다.
void UCombatComponent::RemoveItemAttackBuff()
{
    bIsItemAttackBuffActive    = false;
    IncreaseAttackPower(-ActiveItemAttackBuffAmount);
    ActiveItemAttackBuffAmount = 0.0f;

    ActiveBuffs.RemoveAll([this](const FActiveBuffInfo& Buff)
    {
        return Buff.bIsItemBuff && Buff.BuffID == ActiveItemAttackBuffID;
    });

    ActiveItemAttackBuffID = NAME_None;

    if (OnCombatBuffUpdated.IsBound())
    {
        OnCombatBuffUpdated.Broadcast();
    }
}
