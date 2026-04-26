// =========================================================================================
// ANS_NormalEnemy_BaseAttackTrace.cpp
//
// [파일 역할]
// 일반/엘리트 몬스터의 기본 공격 구간 동안 무기 충돌 트레이스를 수행하는 AnimNotifyState입니다.
// 공격 구간이 시작되면 중복 타격 플래그(bHasDamaged)를 초기화하고,
// 매 프레임 무기 소켓 위치 기준 SphereTrace로 플레이어 충돌을 감지해 데미지를 1회 적용합니다.
//
// [트레이스 대상 우선순위]
// 1. 스태틱 WeaponMesh (무기 메시가 있는 경우 우선 사용)
// 2. 스켈레탈 Weapon 메시 (블루프린트에서 추가된 경우)
// 3. 기본 스켈레탈 메시 (무기가 없는 맨손 공격)
// =========================================================================================

#include "ANS_NormalEnemy_BaseAttackTrace.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy.h"
#include "GameFramework/Pawn.h"

// 공격 구간 시작 시 중복 타격 방지 플래그를 초기화합니다.
void UANS_NormalEnemy_BaseAttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AEnemy* Enemy = Cast<AEnemy>(MeshComp->GetOwner()))
	{
		Enemy->bHasDamaged = false;
	}
}

// 공격 구간 중 매 프레임 무기 소켓 기준 SphereTrace로 플레이어를 감지하고 데미지를 1회 적용합니다.
void UANS_NormalEnemy_BaseAttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	AEnemy* Enemy = Cast<AEnemy>(MeshComp->GetOwner());

	// 이미 이 공격으로 데미지를 줬으면 다시 주지 않습니다.
	if (Enemy == nullptr || Enemy->bHasDamaged) return;

	// 트레이스 기준 컴포넌트를 결정합니다 (스태틱 무기 → 스켈레탈 무기 → 기본 메시 순).
	USceneComponent* TargetComp = MeshComp;

	if (Enemy->WeaponMesh && Enemy->WeaponMesh->GetStaticMesh() != nullptr)
	{
		TargetComp = Enemy->WeaponMesh;
	}
	else
	{
		TArray<USkeletalMeshComponent*> SkeletalMeshes;
		Enemy->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

		for (USkeletalMeshComponent* SkMesh : SkeletalMeshes)
		{
			if (SkMesh != MeshComp && SkMesh->GetName().Contains(TEXT("Weapon")))
			{
				TargetComp = SkMesh;
				break;
			}
		}
	}

	// StartSocketName, EndSocketName은 AnimNotify 에디터에서 설정한 소켓 이름입니다.
	FVector StartLocation = TargetComp->GetSocketLocation(StartSocketName);
	FVector EndLocation = TargetComp->GetSocketLocation(EndSocketName);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Enemy);

	FHitResult HitResult;

	// 플레이어 전용 채널(GameTraceChannel5)로 구체 트레이스를 수행합니다.
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		MeshComp,
		StartLocation, EndLocation, TraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel5), false, ActorsToIgnore,
		EDrawDebugTrace::None, HitResult, true
	);

	if (bHit)
	{
		APawn* HitPawn = Cast<APawn>(HitResult.GetActor());
		if (HitPawn && HitPawn->IsPlayerControlled())
		{
			UGameplayStatics::ApplyDamage(HitPawn, StateDamage, Enemy->GetController(), Enemy, UDamageType::StaticClass());
			Enemy->bHasDamaged = true;
		}
	}
}

// 공격 구간 종료 시 추가 처리가 필요하면 여기에 구현합니다.
void UANS_NormalEnemy_BaseAttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
