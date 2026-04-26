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

	// 경고 데칼이 있으면 그 위치를 XY 기준점으로 사용합니다.
	// 없으면(일반 Enemy) 보스 루트 XY를 폴백으로 사용합니다.
	ABossMonster* Boss = Cast<ABossMonster>(OwnerActor);
	FVector BaseXY = ActorLocation;
	if (Boss && Boss->WarningDecal)
	{
		BaseXY = Boss->WarningDecal->GetComponentLocation();
	}

	// 라인트레이스로 데칼 위치 아래의 정확한 지면 Z를 탐지합니다.
	FVector TraceStart = FVector(BaseXY.X, BaseXY.Y, ActorLocation.Z + 100.f);
	FVector TraceEnd   = FVector(BaseXY.X, BaseXY.Y, ActorLocation.Z - 500.f);

	FHitResult GroundHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);

	bool bHitGround = MeshComp->GetWorld()->LineTraceSingleByChannel(
		GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	// GetActorLocation().Z는 캡슐 중심이므로 HalfHeight를 빼면 실제 발 높이가 됩니다.
	ACharacter* OwnerChar = Cast<ACharacter>(OwnerActor);
	float FloorZ = ActorLocation.Z;
	if (OwnerChar)
	{
		FloorZ = ActorLocation.Z - OwnerChar->GetSimpleCollisionHalfHeight();
	}

	float ImpactZ = bHitGround ? GroundHit.ImpactPoint.Z + 20.f : FloorZ;
	ImpactZ = FMath::Max(ImpactZ, FloorZ);

	// 이펙트와 데미지의 XY 기준점은 플레이어가 본 경고 위치(데칼)와 동일하게 유지합니다.
	FVector ImpactLocation = FVector(BaseXY.X, BaseXY.Y, ImpactZ);

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
		EDrawDebugTrace::None, HitResult, true,
		FLinearColor::Red, FLinearColor::Green, 1.0f);

	// 회피 불가 데미지를 적용합니다.
	if (bHit)
	{
		APawn* HitPawn = Cast<APawn>(HitResult.GetActor());
		if (HitPawn && HitPawn->IsPlayerControlled())
		{
			UGameplayStatics::ApplyDamage(HitPawn, SlamDamage, Enemy->GetController(), Enemy, UUndodgeableDamageType::StaticClass());
			Enemy->bHasDamaged = true;
		}
	}
}
