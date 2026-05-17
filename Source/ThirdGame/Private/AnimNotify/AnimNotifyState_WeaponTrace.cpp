#include "AnimNotify/AnimNotifyState_WeaponTrace.h"
#include "Character/MyCharacter.h" 
#include "Character/CombatComponent.h"

void UAnimNotifyState_WeaponTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (AMyCharacter* Owner = Cast<AMyCharacter>(MeshComp->GetOwner()))
	{
		if (Owner->CombatComp) Owner->CombatComp->StartWeaponTrace(AttackID);
	}
}

void UAnimNotifyState_WeaponTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (AMyCharacter* Owner = Cast<AMyCharacter>(MeshComp->GetOwner()))
	{
		if (Owner->CombatComp) Owner->CombatComp->WeaponTraceTick(); 
	}
}

void UAnimNotifyState_WeaponTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (AMyCharacter* Owner = Cast<AMyCharacter>(MeshComp->GetOwner()))
	{
		if (Owner->CombatComp) Owner->CombatComp->EndWeaponTrace();
	}
}