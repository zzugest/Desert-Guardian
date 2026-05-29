// =========================================================================================
// AN_GroundSlam.cpp
//
// [파일 역할]
// 보스 착지 시 경고 데칼 위치 기준으로 지면 강타 이펙트와 데미지를 적용하는 AnimNotify입니다.
//
// [처리 순서]
// 1. 경고 데칼 위치를 XY 기준점으로 삼아 라인트레이스로 정확한 지면 Z를 탐지
// 2. 경고 데칼 제거
// 3. 나이아가라 이펙트 스폰
// 4. SphereTrace로 범위 내 플레이어 감지 후 회피 불가 데미지 적용
// =========================================================================================

#include "AN_GroundSlam.h"
#include "AnimNotify/AN_AttackBase.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Enemy/Enemy.h"
#include "Enemy/Boss/BossMonster.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/Engine.h"
#include "GlobalUI/UndodgeableDamageType.h"

// 착지 타이밍에 경고 데칼 위치를 기준으로 이펙트와 범위 데미지를 처리합니다.
void UAN_GroundSlam::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	AEnemy* Enemy = Cast<AEnemy>(OwnerActor);
	if (!OwnerActor) return;

	FVector ActorLocation = OwnerActor->GetActorLocation();

	// 캐싱된 히트 기준 위치를 그대로 사용합니다. (데칼 스폰 시 이미 지면 Z가 계산된 위치)
	// 없으면(일반 Enemy 또는 미설정) 보스 발 높이를 폴백으로 사용합니다.
	ABossMonster* Boss = Cast<ABossMonster>(OwnerActor);
	FVector ImpactLocation = ActorLocation;
	if (Boss && !Boss->CachedImpactLocation.IsZero())
	{
		ImpactLocation = Boss->CachedImpactLocation;
	}
	else
	{
		ACharacter* OwnerChar = Cast<ACharacter>(OwnerActor);
		if (OwnerChar)
		{
			ImpactLocation.Z = ActorLocation.Z - OwnerChar->GetSimpleCollisionHalfHeight();
		}
	}

	// 경고 데칼을 제거합니다 (위치를 읽은 뒤 이펙트 생성 직전에 처리).
	if (Boss && Boss->WarningDecal)
	{
		Boss->WarningDecal->DestroyComponent();
		Boss->WarningDecal = nullptr;
	}

	// 나이아가라 강타 이펙트를 스폰합니다.
	if (SlamEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			MeshComp->GetWorld(), SlamEffect,
			ImpactLocation, OwnerActor->GetActorRotation(), EffectScale);
	}

	// 구체 트레이스와 데미지 적용은 서버에서만 처리합니다.
	if (!OwnerActor->HasAuthority()) return;

	// 이미 데미지를 준 경우 구체 트레이스를 생략합니다.
	if (Enemy == nullptr || Enemy->bHasDamaged) return;

	// 구체 트레이스로 범위 내 플레이어를 감지합니다.
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerActor);

	FHitResult HitResult;
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		MeshComp,
		ImpactLocation, ImpactLocation, DamageRadius,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel5),
		false, ActorsToIgnore,
		EDrawDebugTrace::ForDuration, HitResult, true,
		FLinearColor::Red, FLinearColor::Green, 3.0f);

	// 회피 불가 데미지를 적용합니다.
	if (bHit)
	{
		APawn* HitPawn = Cast<APawn>(HitResult.GetActor());
		if (HitPawn && HitPawn->IsPlayerControlled())
		{
			// 히트 리액션 타입을 설정하고 데미지를 적용합니다.
			Enemy->CurrentHitType = HitType;
			UGameplayStatics::ApplyDamage(HitPawn, SlamDamage, Enemy->GetController(), Enemy, UUndodgeableDamageType::StaticClass());
			Enemy->CurrentHitType = FGameplayTag::EmptyTag;
			Enemy->bHasDamaged = true;
		}
	}
}
