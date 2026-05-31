#include "Skill/TornadoProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Character/MyCharacter.h"
#include "Enemy/Enemy.h"

// =========================================================================================
// TornadoProjectile.cpp
//
// [파일 역할]
// 회오리(범위 지속 피해) 투사체 클래스입니다.
// 발사 후 충돌 범위 내의 적에게 일정 주기마다 지속적으로 데미지를 줍니다.
// =========================================================================================

ATornadoProjectile::ATornadoProjectile()
{
	// 이동 속도/수명 기본값 설정
	ProjectileMovement->InitialSpeed = 600.0f;
	ProjectileMovement->MaxSpeed = 600.0f;
	InitialLifeSpan = 5.0f; // 5초 후 자동 소멸
}

void ATornadoProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 1) 부모 클래스에서 바인딩된 '오버랩 시작 데미지' 이벤트를 제거합니다.
	//    이유: 토네이도는 발사 시 단 1회 데미지가 아닌 주기적 데미지를 사용하기 때문입니다.
	if (CollisionComp)
	{
		CollisionComp->OnComponentBeginOverlap.RemoveDynamic(this, &AProjectileBase::OnOverlapBegin);
	}

	// 2) 주기 데미지 타이머 설정:
	//    - DamageInterval 간격마다 ApplyPeriodicDamage를 반복 호출합니다.
	//    - 첫 번째 딜레이(0.0f)는 0이 아닌 값으로 줘야 최소 1회 실행 후 반복됩니다.
	GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &ATornadoProjectile::ApplyPeriodicDamage, DamageInterval, true, 0.0f);
}

void ATornadoProjectile::ApplyPeriodicDamage()
{
	// 클라이언트에서는 실행하지 않습니다. 데미지 판정은 서버에서만 처리합니다.
	if (!HasAuthority()) return;

	// 충돌 컴포넌트가 없으면 아무것도 할 수 없습니다.
	if (!CollisionComp) return;

	// 1) 충돌 범위 안에 겹쳐 있는 액터들 수집
	TArray<AActor*> OverlappingActors;
	CollisionComp->GetOverlappingActors(OverlappingActors);

	// 2) 겹친 액터들을 순회하며 데미지 적용
	for (AActor* Actor : OverlappingActors)
	{
		// 자신 또는 투사체의 발사자(Instigator)는 제외
		if (Actor && Actor != this && Actor != GetInstigator())
		{
			// ApplyDamage:
			// - 대상(Actor)에게 BaseDamage만큼의 데미지를 줍니다.
			// - InstigatorController를 데미지 처리의 출처로 지정하여 데미지 소스가 명확히 표시됩니다.
			UGameplayStatics::ApplyDamage(Actor, BaseDamage, GetInstigatorController(), this, UDamageType::StaticClass());

			AMyCharacter* Caster = Cast<AMyCharacter>(GetInstigator());
			AEnemy* HitEnemy = Cast<AEnemy>(Actor);
			if (Caster && HitEnemy && !HitEnemy->bIsDead)
			{
				// 데미지 텍스트는 Client RPC로 시전자 본인 화면에만 표시합니다.
				Caster->ClientShowDamageText(Actor->GetActorLocation(), BaseDamage);
			}
		}
	}
}