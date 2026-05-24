#include "Skill/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/Enemy.h"
#include "Character/MyCharacter.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// 투사체를 모든 클라이언트에 복제해 비주얼(NiagaraComp)이 모든 화면에 보이도록 합니다.
	// 데미지 판정은 서버에서만 처리합니다.
	bReplicates = true;

	// 1. 콜리전 구체
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(50.0f);
	CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = CollisionComp;

	// 2. 나이아가라 이펙트
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetupAttachment(RootComponent);

	// 3. 이동 컴포넌트 (초기 속도 1000, 중력 0으로 설정)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1000.0f;
	ProjectileMovement->MaxSpeed = 1000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->SetIsReplicated(true); // 서버 이동을 클라이언트에 동기화합니다.

	// 3초 후 자동 소멸 (메모리 절약)
	InitialLifeSpan = 3.0f;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][4] Projectile BeginPlay | Name: %s | Location: %s | HasAuthority: %s"),
		*GetName(), *GetActorLocation().ToString(), HasAuthority() ? TEXT("YES") : TEXT("NO"));

	// 스폰 시점에 시전자를 볼 수 있는 플레이어 컨트롤러를 기록합니다.
	// 이후 IsNetRelevantFor에서 이 목록에 없는 클라이언트에게는 투사체를 복제하지 않습니다.
	if (HasAuthority() && GetInstigator())
	{
		const float CullDistSq = GetInstigator()->NetCullDistanceSquared;
		const FVector CasterLocation = GetInstigator()->GetActorLocation();

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (!PC) continue;
			APawn* ViewPawn = PC->GetPawn();
			if (!ViewPawn) continue;
			const float DistSq = FVector::DistSquared(CasterLocation, ViewPawn->GetActorLocation());
			if (DistSq <= CullDistSq)
			{
				RelevantControllers.Add(PC);
			}
		}
	}

	// 시전자 캐릭터의 캡슐 콜리전에 막히지 않도록 이동 시 무시합니다.
	if (GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
	}

	// 콜리전 함수 등록
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnOverlapBegin);
}

void AProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][5] OnOverlapBegin | OtherActor: %s"),
		OtherActor ? *OtherActor->GetName() : TEXT("NULL"));

	// 플레이어 캐릭터(시전자 및 아군)는 데미지 없이 무시합니다.
	if (Cast<AMyCharacter>(OtherActor)) return;

	// 적(AEnemy)에게만 데미지를 주고 파괴 — MonsterSpawner 등 비전투 액터는 무시
	AEnemy* HitEnemy = Cast<AEnemy>(OtherActor);
	if (OtherActor && OtherActor != this && HitEnemy)
	{
		if (HitEnemy->bIsDead) return; // 사망한 적은 무시

		// 데미지 판정, 이펙트, 데미지 텍스트, 소멸은 서버에서만 처리합니다.
		if (!HasAuthority()) return;

		// 피격 이펙트를 Multicast로 모든 클라이언트 화면에 재생합니다.
		MulticastOnHit(GetActorLocation(), GetActorRotation());

		UGameplayStatics::ApplyDamage(OtherActor, BaseDamage, GetInstigatorController(), this, UDamageType::StaticClass());

		// 데미지 텍스트는 Client RPC로 시전자 본인 화면에만 표시합니다.
		AMyCharacter* Caster = Cast<AMyCharacter>(GetInstigator());
		if (Caster)
		{
			Caster->ClientShowDamageText(OtherActor->GetActorLocation(), BaseDamage);
		}

		// 나이아가라 파티클 회수가 필요하다면 아래 Destroy(); 주석을 풀어주세요.
		// (오브젝트 풀링을 사용한다면 별도 반환 처리가 필요합니다)
		Destroy();
	}
}

void AProjectileBase::MulticastOnHit_Implementation(FVector Location, FRotator Rotation)
{
	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, Location, Rotation);
	}
}

bool AProjectileBase::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	const APlayerController* PC = Cast<APlayerController>(RealViewer);
	if (PC)
	{
		return RelevantControllers.Contains(const_cast<APlayerController*>(PC));
	}
	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}
