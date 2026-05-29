#include "AnimNotify/ANS_SetHitType.h"
#include "Enemy/Enemy.h"

// 공격 구간 시작: 적에게 HitType 태그를 설정합니다.
void UANS_SetHitType::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp) return;

	AEnemy* Enemy = Cast<AEnemy>(MeshComp->GetOwner());
	if (!Enemy) return;

	Enemy->CurrentHitType = HitTag;
}

// 공격 구간 종료: 적의 HitType 태그를 초기화합니다.
void UANS_SetHitType::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp) return;

	AEnemy* Enemy = Cast<AEnemy>(MeshComp->GetOwner());
	if (!Enemy) return;

	Enemy->CurrentHitType = FGameplayTag::EmptyTag;
}
