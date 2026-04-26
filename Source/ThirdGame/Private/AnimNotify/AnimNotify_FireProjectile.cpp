#include "AnimNotify/AnimNotify_FireProjectile.h"
#include "Character/MyCharacter.h"       
#include "Skill/SkillComponent.h"   

void UAnimNotify_FireProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AMyCharacter* Owner = Cast<AMyCharacter>(MeshComp->GetOwner()))
	{
		
		if (Owner->SkillComp)
		{
			Owner->SkillComp->SpawnProjectile(SkillID, SocketName);
		}
	}
}