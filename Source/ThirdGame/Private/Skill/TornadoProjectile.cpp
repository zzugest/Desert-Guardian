#include "Skill/TornadoProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Character/MyCharacter.h"
#include "Enemy/Enemy.h"

// TornadoProjectile.cpp
// Purpose:
//   - ȸ����(���� ������ ����) ����ü ����.
//   - ���� ��� ������ ó�� ��� ���� ������ �ֱ������� �������� ������.
// Key behaviors:
//   - BeginPlay: �θ��� ��� ������ �ݹ��� �����Ͽ� ���� Ÿ���� �����ϰ�,
//                DamageInterval �ֱ�� ApplyPeriodicDamage Ÿ�̸Ӹ� ����.
//   - ApplyPeriodicDamage: �浹 ��ü ������ ��� ���͸� ������ �ֱ��� ������ ����.
// Safety notes:
//   - CollisionComp, GetInstigator(), GetInstigatorController() ���� null�� �� �����Ƿ� ��� �� �˻� �ʿ�.
//   - ApplyDamage ȣ���� ������ ��ġ�� ������ ��Ʈ�ѷ��� �ùٸ��� Ȯ���ؾ� ��.

ATornadoProjectile::ATornadoProjectile()
{
	// �̵� �ӵ�/���� �⺻�� ����
	ProjectileMovement->InitialSpeed = 600.0f;
	ProjectileMovement->MaxSpeed = 600.0f;
	InitialLifeSpan = 5.0f; // 5�� ���� ����
}

void ATornadoProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 1) �θ� Ŭ������ ���ε��� '���ڸ��� ������' ����� ����.
	//    ����: ����̵��� ���� �� ��� 1ȸ ��������, ���� �ֱ� �������� �ߺ� �߻��ϴ� ���� �����ϱ� ����.
	if (CollisionComp)
	{
		CollisionComp->OnComponentBeginOverlap.RemoveDynamic(this, &AProjectileBase::OnOverlapBegin);
	}

	// 2) �ֱ� ������ Ÿ�̸� ����:
	//    - DamageInterval �������� ApplyPeriodicDamage�� �ݺ� ȣ��.
	//    - ������ ����(0.0f)�� 0�� �ƴ� ������ �ָ� ��� 1ȸ ���� �� �ݺ� ����.
	GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &ATornadoProjectile::ApplyPeriodicDamage, DamageInterval, true, 0.0f);
}

void ATornadoProjectile::ApplyPeriodicDamage()
{
	// 클라이언트에서는 실행하지 않습니다. 데미지 판정은 서버에서만 처리합니다.
	if (!HasAuthority()) return;

	// ���: �浹 ������Ʈ�� ������ �ƹ� �͵� �� ��
	if (!CollisionComp) return;

	// 1) �浹 ��ü �ȿ� ���� �ִ� ���͵� ����
	TArray<AActor*> OverlappingActors;
	CollisionComp->GetOverlappingActors(OverlappingActors);

	// 2) ������ ���͵鿡 ���� ��ȸ�ϸ� ������ ����
	for (AActor* Actor : OverlappingActors)
	{
		// �ڽ� �Ǵ� ����ü�� �߻���(Instigator)�� ����
		if (Actor && Actor != this && Actor != GetInstigator())
		{
			// ApplyDamage:
			// - ���(Actor)�� BaseDamage��ŭ�� �������� ����
			// - InstigatorController�� ������ ��Ʈ�ѷ��� �����Ͽ� ������ ��ó ó���� �����ϰ� ��
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