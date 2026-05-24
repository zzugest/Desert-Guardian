#include "AnimNotify/AnimNotify_FireProjectile.h"
#include "Character/MyCharacter.h"       
#include "Skill/SkillComponent.h"   

void UAnimNotify_FireProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AMyCharacter* Owner = Cast<AMyCharacter>(MeshComp->GetOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][2] AnimNotify_FireProjectile fired | Owner: %s | HasAuthority: %s | SkillID: %s"),
			*Owner->GetName(), Owner->HasAuthority() ? TEXT("YES") : TEXT("NO"), *SkillID.ToString());

		// 투사체 생성은 서버에서만 처리합니다. 클라이언트는 애니메이션 시각 재생만 담당합니다.
		if (!Owner->HasAuthority()) return;

		if (Owner->SkillComp)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][2] Calling SpawnProjectile | Socket: %s"), *SocketName.ToString());
			Owner->SkillComp->SpawnProjectile(SkillID, SocketName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][2] SkillComp is NULL on owner %s"), *Owner->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][2] AnimNotify_FireProjectile fired but Owner is NOT AMyCharacter"));
	}
}