// =========================================================================================
// ANS_ChangePlayRate.cpp
//
// [파일 역할]
// 애니메이션 구간 동안 몽타주 재생 속도를 동적으로 변경하는 AnimNotifyState입니다.
// 보스 공격 애니메이션의 특정 구간(예: 예비 동작)을 느리게 하거나 빠르게 할 때 사용합니다.
// 구간 시작 시 TargetPlayRate로 변경, 구간 종료 시 1.0으로 복원합니다.
// =========================================================================================

#include "ANS_ChangePlayRate.h"

// 구간 시작 시 현재 활성 몽타주의 재생 속도를 TargetPlayRate로 변경합니다.
void UANS_ChangePlayRate::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
	{
		if (UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage())
		{
			AnimInst->Montage_SetPlayRate(CurrentMontage, TargetPlayRate);
		}
	}
}

// 구간 종료 시 재생 속도를 기본값(1.0)으로 복원합니다.
void UANS_ChangePlayRate::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
	{
		if (UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage())
		{
			AnimInst->Montage_SetPlayRate(CurrentMontage, 1.0f);
		}
	}
}
