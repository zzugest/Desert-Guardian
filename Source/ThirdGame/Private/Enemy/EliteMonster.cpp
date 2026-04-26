// =========================================================================================
// EliteMonster.cpp
//
// [파일 역할]
// AEnemy를 상속받는 엘리트 몬스터 클래스입니다.
// BeginPlay에서 갑옷 메시(기본 메시 및 무기 외의 모든 스켈레탈 메시)를
// SetLeaderPoseComponent로 기본 메시에 연결해 애니메이션을 통일합니다.
// ExecuteAttackHit에서 사거리 내 플레이어에게 직접 데미지를 적용합니다.
// =========================================================================================

#include "EliteMonster.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BlackboardComponent.h"

// 부모(AEnemy)의 기본값(Tick 활성화 등)을 유지합니다.
AEliteMonster::AEliteMonster()
{
	// AEnemy::Tick이 HP바 카메라 방향 빌보딩을 처리하므로 Tick이 필요합니다.
}

// 스폰 시 갑옷 메시들을 기본 메시(Leader)에 연결해 한 세트의 애니메이션으로 구동합니다.
void AEliteMonster::BeginPlay()
{
	Super::BeginPlay();

	TArray<USkeletalMeshComponent*> AllMeshes;
	GetComponents<USkeletalMeshComponent>(AllMeshes);

	for (USkeletalMeshComponent* ArmorMesh : AllMeshes)
	{
		// 기본 메시 자신이거나 무기 메시는 제외하고 갑옷 메시만 Leader에 연결합니다.
		if (!ArmorMesh || ArmorMesh == GetMesh() || ArmorMesh->GetName().Contains(TEXT("Weapon"))) continue;

		// 갑옷 메시가 기본 메시의 애니메이션 포즈를 따라가도록 연결합니다.
		ArmorMesh->SetLeaderPoseComponent(GetMesh());
	}
}

// 공격 애니메이션 타이밍에 사거리 내 플레이어에게 데미지를 적용합니다.
void AEliteMonster::ExecuteAttackHit()
{
	if (!TargetPlayer || CurrentHP <= 0.0f) return;

	float DistanceToPlayer = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

	// 애니메이션 재생 중 플레이어가 사거리 밖으로 이동한 경우 데미지를 넣지 않습니다.
	if (DistanceToPlayer > AttackRange + 50.0f) return;

	UGameplayStatics::ApplyDamage(TargetPlayer, AttackPower, GetController(), this, UDamageType::StaticClass());
}
