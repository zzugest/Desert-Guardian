#include "Skill/SkillComponent.h"
#include "Skill/SkillSubsystem.h"
#include "Skill/SkillData.h"
#include "CombatComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "MyCharacter.h"
#include "Character/TargetingComponent.h"
#include "Skill/ProjectileBase.h"
#include "WarningSubsystem.h"

// SkillComponent.cpp
// Purpose:
//   - ĳ���Ϳ� �ٴ� ��ų ����/��ٿ�/���� ���� ������Ʈ.
//   - ��ų ���� �� ��ٿ� ���, ���� ����, ���� ���, ����ü ���� ���� ���.
// Key behaviors:
//   - BeginPlay: SkillSubsystem���� ��� ������ ���� �� ���� ����Ʈ ���� ��ε�ĳ��Ʈ.
//   - TickComponent: ActiveCooldowns/ActiveBuffs �ð� ��� ó�� �� ���� �� ����/��ε�ĳ��Ʈ.
//   - TryCastSkill: ��ų ��� ������(��ٿ�/���� üũ, �ִϸ��̼� ���, ���� ����, ����ü ��ȯ).
//   - AddBuff/Revive/SaveTemporaryData �� ���� �Լ���.
// Safety notes:
//   - SkillSubsystem, SkillData, CombatComp ������ null üũ �ʼ�.
//   - ActiveCooldowns ��ȸ/������ Iterator ���, ActiveBuffs ������ ���� ó���� �����ϰ� ����.

USkillComponent::USkillComponent()
{
	// ��Ÿ�� ó���� ���� ƽ ���
	PrimaryComponentTick.bCanEverTick = true;
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	// �������κ��� CombatComp ĳ�� (���� ����)
	if (AActor* Owner = GetOwner())
	{
		CombatComp = Owner->FindComponentByClass<UCombatComponent>();
	}

	// SkillSubsystem���� ��� ������ ����
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>())
		{
			SkillSys->LoadTemporaryData(ActiveCooldowns, ActiveBuffs);

			// ������ ������ ������ UI ���� �˸�
			if (ActiveBuffs.Num() > 0)
			{
				OnBuffListUpdated.Broadcast();
			}
		}
	}

	bool bIsServer = GetOwner() && GetOwner()->HasAuthority();
	UE_LOG(LogTemp, Warning, TEXT("[LV_TRAVEL] SkillComp BeginPlay | Auth:%s | Cooldowns:%d | Buffs:%d"),
		bIsServer ? TEXT("SERVER") : TEXT("CLIENT"),
		ActiveCooldowns.Num(), ActiveBuffs.Num());
	for (const FActiveBuff& Buff : ActiveBuffs)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LV_TRAVEL]   - SkillBuff: %s | Remaining:%.1fs"),
			*Buff.BuffID.ToString(), Buff.RemainingTime);
	}
}

void USkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// -----------------------
	// 1) ��Ÿ�� ���� (TMap)
	// -----------------------
	if (ActiveCooldowns.Num() > 0)
	{
		for (auto It = ActiveCooldowns.CreateIterator(); It; ++It)
		{
			It.Value() -= DeltaTime;
			if (It.Value() <= 0.0f)
			{
				It.RemoveCurrent();
			}
		}
	}

	// -----------------------
	// 2) ���� ���� (TArray, ���� ����)
	// -----------------------
	if (ActiveBuffs.Num() > 0)
	{
		bool bBuffExpired = false;

		for (int32 i = ActiveBuffs.Num() - 1; i >= 0; --i)
		{
			ActiveBuffs[i].RemainingTime -= DeltaTime;

			if (ActiveBuffs[i].RemainingTime <= 0.0f)
			{
				//UE_LOG(LogTemp, Log, TEXT("���� ����: %s"), *ActiveBuffs[i].BuffID.ToString());
				ActiveBuffs.RemoveAt(i);
				bBuffExpired = true;
			}
		}

		if (bBuffExpired)
		{
			OnBuffListUpdated.Broadcast();
		}
	}
}

void USkillComponent::TryCastSkill(int32 SlotIndex)
{
	// 0) 스킬 시전 불가 상태 체크 (공격 중, 마법 시전 중, 구르기 중, 스턴)
	AMyCharacter* StateOwner = Cast<AMyCharacter>(GetOwner());
	if (StateOwner)
	{
		if (StateOwner->HasStateTag("State.Action.Attacking") ||
			StateOwner->HasStateTag("State.Action.MagicCasting") ||
			StateOwner->HasStateTag("State.Action.Rolling") ||
			StateOwner->HasStateTag("State.CC.Stun"))
		{
			return;
		}
	}

	// 1) SkillSubsystem ȹ��
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return;

	USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
	if (!SkillSys) return;

	// 2) ������ ��ų ID Ȯ��
	FName SkillID = SkillSys->GetSkillIDInSlot(SlotIndex);
	if (SkillID.IsNone())
	{
		return;
	}

	// 3) ���� ��Ÿ�� ������ �˻�
	if (ActiveCooldowns.Contains(SkillID))
	{
		float Remaining = ActiveCooldowns[SkillID];

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
			WarningSys->ShowWarning(FText::Format(FormatPattern, FText::AsNumber(Remaining, &NumFormat)));
		}
		return;
	}

	// 4) ��ų ������ ��ȸ
	FSkillData* Data = SkillSys->GetSkillData(SkillID);
	if (!Data) return;

	// 5) MP 잔액 클라이언트 사전 검사 (UX용 — 실제 차감은 서버에서 처리)
	if (CombatComp)
	{
		if (CombatComp->CurrentMP < Data->ManaCost)
		{
			UWarningSubsystem* WarningSys = GI->GetSubsystem<UWarningSubsystem>();
			if (WarningSys)
			{
				FText WarningText = FText::FromStringTable(
					TEXT("/Game/character/ST_WarningMessages.ST_WarningMessages"),
					TEXT("Err_NotEnoughMP")
				);
				WarningSys->ShowWarning(WarningText);
			}
			return;
		}
	}

	// 6) 클라이언트에서 애니메이션 로컬 재생 (시각적 피드백) 및 쿨다운 등록
	AMyCharacter* MyChar = Cast<AMyCharacter>(GetOwner());
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character && Data->SkillMontage)
	{
		// 스킬 시전 중 다른 행동을 막기 위해 MagicCasting 태그를 추가합니다.
		if (MyChar)
		{
			MyChar->AddStateTag("State.Action.MagicCasting");
		}

		UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance();
		Character->PlayAnimMontage(Data->SkillMontage);
		ActiveCooldowns.Add(SkillID, Data->Cooldown);

		// 몽타주 종료 시 MagicCasting 태그를 제거하는 콜백을 등록합니다.
		if (AnimInst)
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &USkillComponent::OnSkillMontageEnded);
			AnimInst->Montage_SetEndDelegate(EndDelegate, Data->SkillMontage);
		}

		// 버프 스킬이면 UI 버프 아이콘만 로컬 등록합니다. 실제 스탯 적용은 서버에서 처리합니다.
		if (Data->SkillType == ESkillType::Buff)
		{
			AddBuff(SkillID, Data->Duration);

			if (Data->BuffEffect)
			{
				UNiagaraFunctionLibrary::SpawnSystemAttached(
					Data->BuffEffect,
					Character->GetMesh(),
					NAME_None,
					FVector(0.f, 0.f, -90.f),
					FRotator::ZeroRotator,
					EAttachLocation::KeepRelativeOffset,
					true
				);
			}
		}
	}

	// 7) 실제 MP 차감과 투사체 생성·버프 스탯 적용은 서버 RPC로 위임합니다.
	if (MyChar)
	{
		MyChar->ServerCastSkill(SkillID);
	}
}

float USkillComponent::GetRemainingCooldown(FName SkillID)
{
	if (ActiveCooldowns.Contains(SkillID))
	{
		return ActiveCooldowns[SkillID];
	}
	return 0.0f;
}

void USkillComponent::RegistSkillToSlot(int32 SlotIndex, FName SkillID)
{
	QuickSlots.Add(SlotIndex, SkillID);
}

FName USkillComponent::GetSkillIDAtSlot(int32 SlotIndex)
{
	if (QuickSlots.Contains(SlotIndex))
	{
		return QuickSlots[SlotIndex];
	}
	return NAME_None;
}

float USkillComponent::GetMaxCooldown(FName SkillID) const
{
	// ������ ��ȸ�� ���� SkillSubsystem ���
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return 1.0f;

	USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
	if (!SkillSys) return 1.0f;

	FSkillData* SkillData = SkillSys->GetSkillData(SkillID);
	if (SkillData)
	{
		return SkillData->Cooldown;
	}

	return 1.0f;
}

void USkillComponent::AddBuff(FName NewBuffID, float Duration)
{
	// ���� ���� ������ ������ ���ӽð� ���� �� UI ���� ���
	for (FActiveBuff& Buff : ActiveBuffs)
	{
		if (Buff.BuffID == NewBuffID)
		{
			Buff.RemainingTime = Duration;
			Buff.MaxDuration = Duration;
			OnBuffListUpdated.Broadcast();
			return;
		}
	}

	// �� ���� �߰� �� UI ���� ���
	FActiveBuff NewBuff;
	NewBuff.BuffID = NewBuffID;
	NewBuff.RemainingTime = Duration;
	NewBuff.MaxDuration = Duration;

	ActiveBuffs.Add(NewBuff);
	OnBuffListUpdated.Broadcast();
}

void USkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ���� ��ȯ �Ǵ� ���� �� ��� ����
	if (EndPlayReason == EEndPlayReason::LevelTransition || EndPlayReason == EEndPlayReason::RemovedFromWorld || EndPlayReason == EEndPlayReason::Destroyed)
	{
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			if (USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>())
			{
				SkillSys->SaveTemporaryData(ActiveCooldowns, ActiveBuffs);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void USkillComponent::SpawnProjectile(FName SkillID, FName SocketName)
{
	AMyCharacter* Owner = Cast<AMyCharacter>(GetOwner());
	if (!Owner || !Owner->CombatComp) return;

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return;

	USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
	if (!SkillSys) return;

	FSkillData* Data = SkillSys->GetSkillData(SkillID);
	UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][3] SpawnProjectile called | SkillID: %s | DataFound: %s"),
		*SkillID.ToString(), Data ? TEXT("YES") : TEXT("NO"));

	if (Data && Data->SkillType == ESkillType::Attack && Data->ProjectileClass)
	{
		float CurrentBaseAtk = Owner->CombatComp->BaseAttackPower;
		float FinalDamage = CurrentBaseAtk * Data->DamageMultiplier;

		FVector SpawnLocation = Owner->GetMesh()->GetSocketLocation(SocketName);

		UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][3] Spawning projectile | Socket: %s | Location: %s | Damage: %f"),
			*SocketName.ToString(), *SpawnLocation.ToString(), FinalDamage);

		// 타겟이 있으면 타겟 방향으로, 없으면 캐릭터 정면으로 발사
		FRotator SpawnRotation = Owner->GetActorRotation();
		if (Owner->TargetingComp && Owner->TargetingComp->CurrentTarget)
		{
			FVector TargetLocation = Owner->TargetingComp->CurrentTarget->GetActorLocation();
			SpawnRotation = (TargetLocation - SpawnLocation).Rotation();
			SpawnRotation.Pitch = 0.f; // 수직 각도 제거 — 높이 차이로 인한 대각선 비행 방지
		}

		// ��ȯ �� ����/�ν�Ƽ�����͸� �����Ͽ� ������ ��ó�� ����
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = Owner;
		SpawnParams.Owner = Owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Data->ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

		AProjectileBase* Projectile = Cast<AProjectileBase>(SpawnedActor);
		if (Projectile)
		{
			Projectile->BaseDamage = FinalDamage;
			UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][3] Projectile spawned successfully | BaseDamage: %f"), FinalDamage);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][3] SpawnActor FAILED or Cast failed | SpawnedActor: %s"),
				SpawnedActor ? *SpawnedActor->GetName() : TEXT("NULL"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][3] Condition not met | SkillType: %s | ProjectileClass: %s"),
			Data ? (Data->SkillType == ESkillType::Attack ? TEXT("Attack") : TEXT("Other")) : TEXT("NoData"),
			(Data && Data->ProjectileClass) ? TEXT("Valid") : TEXT("NULL"));
	}
}

void USkillComponent::OnSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 스킬 애니메이션이 끝나면 MagicCasting 태그를 제거해 다른 행동을 다시 허용합니다.
	if (AMyCharacter* Owner = Cast<AMyCharacter>(GetOwner()))
	{
		Owner->RemoveStateTag("State.Action.MagicCasting");
	}
}