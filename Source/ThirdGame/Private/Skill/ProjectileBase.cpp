#include "Skill/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/Enemy.h"
#include "Character/MyCharacter.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1. �浹ü ����
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(50.0f);
	CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = CollisionComp;

	// 2. ����Ʈ ����
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetupAttachment(RootComponent);

	// 3. �̵� ���� (�ʱ� �ӵ� 1000, �߷� 0���� ����)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1000.0f;
	ProjectileMovement->MaxSpeed = 1000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// 3�� �� �ڵ� �Ҹ� (�޸� ����)
	InitialLifeSpan = 3.0f;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// �浹 �Լ� ����
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnOverlapBegin);
}

void AProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// �� �ڽ��� �ƴϰ�, ���� �� ���ε� �ƴ� ���� Ÿ��
	// 적(AEnemy)에게만 데미지를 주고 파괴 — MonsterSpawner 등 비전투 액터는 무시
	AEnemy* HitEnemy = Cast<AEnemy>(OtherActor);
	if (OtherActor && OtherActor != this && HitEnemy)
	{
		if (HitEnemy->bIsDead) return; // 사망한 적은 무시

		UGameplayStatics::ApplyDamage(OtherActor, BaseDamage, GetInstigatorController(), this, UDamageType::StaticClass());

		AMyCharacter* Caster = Cast<AMyCharacter>(GetInstigator());
		if (Caster)
		{
			Caster->OnSpawnDamageText(OtherActor->GetActorLocation(), BaseDamage);
		}

		if (HitEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, GetActorLocation(), GetActorRotation());
		}

		//UE_LOG(LogTemp, Warning, TEXT(">>����ü ����: [%s] / ������: %f"), *OtherActor->GetName(), BaseDamage);

		// ���̾ó�� �°� ������ �Ϸ��� �Ʒ� Destroy(); �ּ��� Ǯ���ּ���.
		// (���� ȸ������ �����ؾ� �ϴ� �ϴ� �ּ� ó���� �Ӵϴ�)
		Destroy(); 
	}
}