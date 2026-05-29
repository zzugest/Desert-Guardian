// =========================================================================================
// AN_GroundSlamSocket.cpp
//
// [파일 역할]
// 보스 강타 공격에서 무기 소켓 위치를 기준으로 이펙트와 데미지를 적용하는 AnimNotify입니다.
// AN_GroundSlam과 달리 XY 기준점을 무기 소켓(검 끝 등)으로 사용합니다.
//
// [처리 순서]
// 1. 무기 소켓 위치 XY + 라인트레이스로 정확한 지면 Z 탐지
// 2. 나이아가라 이펙트 스폰
// 3. SphereTrace로 범위 내 플레이어 감지 후 회피 불가 데미지 적용
// =========================================================================================

#include "AN_GroundSlamSocket.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Enemy/Enemy.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GlobalUI/UndodgeableDamageType.h"

// 무기 소켓 위치를 XY 기준으로 지면 강타 이펙트와 범위 데미지를 처리합니다.
void UAN_GroundSlamSocket::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	AEnemy* Enemy = Cast<AEnemy>(OwnerActor);
	if (!OwnerActor) return;

	// 무기 메시가 있으면 그것의 소켓을, 없으면 기본 스켈레탈 메시 소켓을 사용합니다.
	USceneComponent* TargetComp = MeshComp;
	if (Enemy && Enemy->WeaponMesh)
	{
		TargetComp = Enemy->WeaponMesh;
	}

	FVector SocketLocation = TargetComp->GetSocketLocation(SocketName);
	FVector ActorLocation  = OwnerActor->GetActorLocation();

	// 보스 루트 XY 기준으로 라인트레이스해 지면 Z를 탐지합니다.
	FVector TraceStart = FVector(ActorLocation.X, ActorLocation.Y, ActorLocation.Z + 100.f);
	FVector TraceEnd   = FVector(ActorLocation.X, ActorLocation.Y, ActorLocation.Z - 500.f);

	FHitResult GroundHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);

	bool bHitGround = MeshComp->GetWorld()->LineTraceSingleByChannel(
		GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	// 캡슐 중심에서 HalfHeight를 빼면 실제 발 높이가 됩니다.
	ACharacter* OwnerChar = Cast<ACharacter>(OwnerActor);
	float FloorZ = ActorLocation.Z;
	if (OwnerChar)
	{
		FloorZ = ActorLocation.Z - OwnerChar->GetSimpleCollisionHalfHeight();
	}

	float ImpactZ = bHitGround ? GroundHit.ImpactPoint.Z + 20.f : FloorZ;
	ImpactZ = FMath::Max(ImpactZ, FloorZ);

	// 이펙트 XY는 소켓(검 끝) 위치, Z는 보정된 지면 높이를 사용합니다.
	FVector ImpactLocation = FVector(SocketLocation.X, SocketLocation.Y, ImpactZ);

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
			// 히트 리액션 타입을 설정하고 데미지를 적용합니다.
			Enemy->CurrentHitType = HitType;
			UGameplayStatics::ApplyDamage(HitPawn, SlamDamage, Enemy->GetController(), Enemy, UUndodgeableDamageType::StaticClass());
			Enemy->CurrentHitType = FGameplayTag::EmptyTag;
			Enemy->bHasDamaged = true;
		}
	}
}
