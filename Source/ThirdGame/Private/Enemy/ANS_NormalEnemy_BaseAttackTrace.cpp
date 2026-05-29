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

	// 데미지 판정은 서버에서만 수행합니다.
	if (!Enemy->HasAuthority()) return;

	// 트레이스 기준 컴포넌트를 결정합니다.
	// 스태틱 WeaponMesh가 있으면 사용(일반 몬스터), 없으면 메인 스켈레탈 메시 사용(정예 몬스터).
	// 정예 몬스터는 무기가 메인 스켈레톤에 통합되어 있으며,
	// 소켓 이름은 각 공격 애니메이션의 ANS에 몬스터별로 개별 지정됩니다.
	USceneComponent* TargetComp = MeshComp;

	if (Enemy->WeaponMesh && Enemy->WeaponMesh->GetStaticMesh() != nullptr)
	{
		TargetComp = Enemy->WeaponMesh;
	}

	// StartSocketName, EndSocketName은 AnimNotify 에디터에서 설정한 소켓 이름입니다.
	FVector StartLocation = TargetComp->GetSocketLocation(StartSocketName);
	FVector EndLocation = TargetComp->GetSocketLocation(EndSocketName);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Enemy);

	FHitResult HitResult;

	UE_LOG(LogTemp, Warning, TEXT("[ANS_DBG] NotifyTick | Enemy: %s | Auth: %s | Start: %s | End: %s"),
		*Enemy->GetName(),
		Enemy->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
		*StartLocation.ToString(), *EndLocation.ToString());

	// 플레이어 전용 채널(GameTraceChannel5)로 구체 트레이스를 수행합니다.
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		MeshComp,
		StartLocation, EndLocation, TraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel5), false, ActorsToIgnore,
		EDrawDebugTrace::ForDuration, HitResult, true, FLinearColor::Blue, FLinearColor::Red, 0.3f
	);

	if (bHit)
	{
		APawn* HitPawn = Cast<APawn>(HitResult.GetActor());
		if (HitPawn && HitPawn->IsPlayerControlled())
		{
			UE_LOG(LogTemp, Warning, TEXT("[ANS_DBG] HIT | Target: %s | Damage: %.1f | Auth: %s"),
				*HitPawn->GetName(), StateDamage,
				Enemy->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));

			Enemy->CurrentHitType = HitType;
			UGameplayStatics::ApplyDamage(HitPawn, StateDamage, Enemy->GetController(), Enemy, UDamageType::StaticClass());
			Enemy->CurrentHitType = FGameplayTag::EmptyTag;
			Enemy->bHasDamaged = true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANS_DBG] MISS | Auth: %s"),
			Enemy->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
	}
}

// 공격 구간 종료 시 추가 처리가 필요하면 여기에 구현합니다.
void UANS_NormalEnemy_BaseAttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
