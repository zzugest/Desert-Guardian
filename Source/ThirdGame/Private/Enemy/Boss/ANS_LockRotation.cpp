// =========================================================================================
// ANS_LockRotation.cpp
//
// [파일 역할]
// 애니메이션 구간 동안 캐릭터의 회전을 잠그는 AnimNotifyState입니다.
// 보스의 특정 공격 모션 중 AI와 컨트롤러의 자동 회전을 비활성화해
// 애니메이션이 의도된 방향을 유지하도록 합니다.
// 구간 종료 시 컨트롤러 기반 회전을 복원합니다.
// =========================================================================================

#include "ANS_LockRotation.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"

// 구간 시작 시 AI 포커스와 컨트롤러 자동 회전을 모두 해제해 회전을 고정합니다.
void UANS_LockRotation::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (ACharacter* Char = Cast<ACharacter>(MeshComp->GetOwner()))
	{
		// 컨트롤러 Yaw 회전을 끊어 애니메이션 방향을 고정합니다.
		Char->bUseControllerRotationYaw = false;

		if (UCharacterMovementComponent* Movement = Char->GetCharacterMovement())
		{
			Movement->bUseControllerDesiredRotation = false;
			Movement->bOrientRotationToMovement = false;
		}

		// AI 포커스를 모든 우선순위에서 해제합니다.
		if (AAIController* AIC = Cast<AAIController>(Char->GetController()))
		{
			AIC->ClearFocus(EAIFocusPriority::Gameplay);
			AIC->ClearFocus(EAIFocusPriority::Move);
			AIC->ClearFocus(EAIFocusPriority::Default);
		}
	}
}

// 구간 종료 시 컨트롤러 기반 부드러운 회전을 복원합니다.
void UANS_LockRotation::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (ACharacter* Char = Cast<ACharacter>(MeshComp->GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = Char->GetCharacterMovement())
		{
			Movement->bUseControllerDesiredRotation = true;
		}
	}
}
