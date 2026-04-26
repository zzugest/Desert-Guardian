// =========================================================================================
// ANS_TrackTarget.cpp
//
// [파일 역할]
// 애니메이션 구간 동안 캐릭터가 플레이어를 부드럽게 추적해 바라보도록 하는 AnimNotifyState입니다.
// 보스의 공격 선딜 구간에 사용해, 플레이어가 움직여도 공격 방향을 실시간으로 보정합니다.
// 구간 종료 시 AI 포커스와 회전 설정을 원래대로 복원합니다.
// =========================================================================================

#include "ANS_TrackTarget.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

// 구간 시작 시 AI 포커스를 플레이어로 설정하고 컨트롤러 방향으로 부드럽게 회전하도록 활성화합니다.
void UANS_TrackTarget::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (ACharacter* Char = Cast<ACharacter>(MeshComp->GetOwner()))
	{
		// 컨트롤러 Yaw로 즉시 스냅되도록 활성화합니다.
		Char->bUseControllerRotationYaw = true;

		if (UCharacterMovementComponent* Movement = Char->GetCharacterMovement())
		{
			Movement->bUseControllerDesiredRotation = true;
			Movement->RotationRate = FRotator(0.0f, TrackingSpeed, 0.0f);
		}

		// AI가 0번 플레이어(로컬 캐릭터)를 포커스 타겟으로 설정합니다.
		if (AAIController* AIC = Cast<AAIController>(Char->GetController()))
		{
			if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(Char->GetWorld(), 0))
			{
				AIC->SetFocus(PlayerPawn, EAIFocusPriority::Gameplay);
			}
		}
	}
}

// 구간 종료 시 AI 포커스를 해제하고 회전 설정을 기본값으로 복원합니다.
void UANS_TrackTarget::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (ACharacter* Char = Cast<ACharacter>(MeshComp->GetOwner()))
	{
		Char->bUseControllerRotationYaw = false;

		if (UCharacterMovementComponent* Movement = Char->GetCharacterMovement())
		{
			Movement->bUseControllerDesiredRotation = false;
			Movement->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
		}

		if (AAIController* AIC = Cast<AAIController>(Char->GetController()))
		{
			AIC->ClearFocus(EAIFocusPriority::Gameplay);
		}
	}
}
