// =========================================================================================
// BTTask_TurnToPlayer.cpp
//
// [파일 역할]
// AI 비헤이비어 트리에서 보스가 플레이어 방향으로 회전하도록 하는 커스텀 BTTask입니다.
// 각도 차이(DeltaYaw)에 따라 알맞은 회전 몽타주(90°/180° 좌·우)를 선택해 재생하고,
// 몽타주가 끝날 때 LatentTask를 완료 처리합니다.
// 루트 모션(Root Motion)이 실제 회전을 담당하므로 TickTask에서는 별도 처리를 하지 않습니다.
// =========================================================================================

#include "BTTask_TurnToPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Enemy/Boss/BossMonster.h"

// 노드 이름을 설정하고, TickTask 활성화 및 블랙보드 키 타입을 초기화합니다.
UBTTask_TurnToPlayer::UBTTask_TurnToPlayer()
{
	NodeName = TEXT("Turn To Player (Root Motion)");
	bNotifyTick = true;
	bIsTurning = false;
	CachedOwnerComp = nullptr;

	// 블랙보드 키를 AActor 타입으로 필터링합니다.
	PlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_TurnToPlayer, PlayerKey), AActor::StaticClass());
}

// 플레이어와의 각도 차이를 계산해 적절한 회전 몽타주를 선택하고 재생합니다.
EBTNodeResult::Type UBTTask_TurnToPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedOwnerComp = &OwnerComp;
	bIsTurning = false;

	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BBComp) return EBTNodeResult::Failed;

	// 블랙보드에서 플레이어 레퍼런스를 가져옵니다.
	AActor* Player = Cast<AActor>(BBComp->GetValueAsObject(PlayerKey.SelectedKeyName));
	ACharacter* BossChar = Cast<ACharacter>(AICon->GetPawn());
	if (!Player || !BossChar) return EBTNodeResult::Failed;

	// Z축 차이를 무시하고 수평 평면에서 플레이어 방향과 보스 현재 방향의 각도 차이를 계산합니다.
	FVector ToPlayer = Player->GetActorLocation() - BossChar->GetActorLocation();
	ToPlayer.Z = 0.0f;

	if (ToPlayer.IsNearlyZero()) return EBTNodeResult::Succeeded;

	FRotator LookAtRot = ToPlayer.Rotation();
	FRotator BossRot   = BossChar->GetActorRotation();

	// -180 ~ 180° 범위로 정규화된 각도 차이를 구합니다.
	float DeltaYaw = FMath::FindDeltaAngleDegrees(BossRot.Yaw, LookAtRot.Yaw);

	// 15° 이내이면 이미 충분히 바라보고 있으므로 즉시 성공 처리합니다.
	if (FMath::Abs(DeltaYaw) < 15.0f)
	{
		return EBTNodeResult::Succeeded;
	}

	// 각도 범위에 따라 재생할 회전 몽타주를 선택합니다.
	UAnimMontage* SelectedMontage = nullptr;

	if      (DeltaYaw >= 45.0f  && DeltaYaw  < 135.0f)  SelectedMontage = RotateMontages.TurnR90;
	else if (DeltaYaw <= -45.0f && DeltaYaw  > -135.0f) SelectedMontage = RotateMontages.TurnL90;
	else if (DeltaYaw >= 135.0f || DeltaYaw  <= -135.0f)
	{
		SelectedMontage = (DeltaYaw > 0.0f) ? RotateMontages.TurnR180 : RotateMontages.TurnL180;
	}

	// 선택된 몽타주가 없으면 즉시 완료 처리합니다.
	if (!SelectedMontage) return EBTNodeResult::Succeeded;

	// Multicast로 모든 클라이언트에 회전 몽타주를 동기화하고, 종료 델리게이트는 서버에만 바인딩합니다.
	ABossMonster* Boss = Cast<ABossMonster>(BossChar);
	if (Boss)
	{
		Boss->Multicast_PlayTurnMontage(SelectedMontage);

		UAnimInstance* AnimInstance = BossChar->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UBTTask_TurnToPlayer::OnTurnMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, SelectedMontage);
		}

		bIsTurning = true;
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

// 루트 모션이 회전을 처리하므로 TickTask에서는 별도 처리를 하지 않습니다.
void UBTTask_TurnToPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
}

// 몽타주 재생이 완료되면 비헤이비어 트리에 회전 완료를 알립니다.
void UBTTask_TurnToPlayer::OnTurnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsTurning = false;

	if (CachedOwnerComp)
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	}
}
