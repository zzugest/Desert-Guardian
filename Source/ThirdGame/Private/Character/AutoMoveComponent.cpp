// =========================================================================================
// AutoMoveComponent.cpp
//
// [파일 역할]
// 플레이어 캐릭터에 붙어 NavMesh 경로 기반 자동이동을 처리하는 컴포넌트입니다.
// - StartAutoMove(Destination) 호출 시 NavMesh로 경로를 계산하고 지면에 데칼 선을 표시합니다.
// - Tick마다 AddMovementInput()으로 다음 웨이포인트를 향해 이동합니다.
// - 1초마다 경로를 재계산해 몬스터 우회(NavMesh Dynamic Obstacle) 및 지형 변화를 반영합니다.
// - 제자리 누적 시간이 StuckMaxTime을 초과하거나 목적지에 도달하면 자동으로 중단합니다.
// - 로컬 컨트롤 캐릭터에서만 작동하므로 서버 / 원격 클라이언트에는 영향을 주지 않습니다.
// =========================================================================================

#include "AutoMoveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAutoMoveComponent::UAutoMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// 자동이동 중에만 Tick이 돌도록 기본값을 꺼 둡니다.
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UAutoMoveComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAutoMoveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAutoMove();
	Super::EndPlay(EndPlayReason);
}

// 지정한 위치를 향해 경로를 계산하고 자동이동을 시작합니다.
void UAutoMoveComponent::StartAutoMove(FVector TargetLocation)
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar || !OwnerChar->IsLocallyControlled()) return;

	Destination        = TargetLocation;
	UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 원본 목적지: %s"), *Destination.ToString());

	// 목적지 위에서 아래로 트레이스해 실제 지면 Z를 구합니다.
	FHitResult HitResult;
	FVector TraceStart = FVector(Destination.X, Destination.Y, Destination.Z + 500.f);
	FVector TraceEnd   = FVector(Destination.X, Destination.Y, Destination.Z - 500.f);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
	{
		Destination.Z = HitResult.ImpactPoint.Z;
		UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 트레이스 성공 → 보정된 목적지: %s"), *Destination.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 트레이스 실패 → 원본 Z 그대로 사용: %.1f"), Destination.Z);
	}

	UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 플레이어 위치: %s"), *OwnerChar->GetActorLocation().ToString());

	UNavigationSystemV1* NavSysCheck = UNavigationSystemV1::GetCurrent(GetWorld());
	UE_LOG(LogTemp, Warning, TEXT("[AutoMove] NavSys: %s"), NavSysCheck ? TEXT("유효") : TEXT("null"));
	if (NavSysCheck)
	{
		FNavLocation ProjectedStart, ProjectedEnd;
		bool bStartOk = NavSysCheck->ProjectPointToNavigation(OwnerChar->GetActorLocation(), ProjectedStart, FVector(200.f, 200.f, 200.f));
		bool bEndOk   = NavSysCheck->ProjectPointToNavigation(Destination,                   ProjectedEnd,   FVector(500.f, 500.f, 500.f));
		UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 시작점 NavMesh: %s / 목적지 NavMesh: %s"),
			bStartOk ? TEXT("O") : TEXT("X"), bEndOk ? TEXT("O") : TEXT("X"));
	}

	CurrentPathIndex   = 0;
	StuckTimer         = 0.0f;
	StuckCheckAccumulator = 0.0f;
	LastStuckCheckPos  = OwnerChar->GetActorLocation();

	// 초기 경로 계산. 경로를 찾지 못하면 시작하지 않습니다.
	RecalcPath();
	if (PathPoints.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 경로 계산 실패 - NavMesh가 목적지까지 생성됐는지 확인. PathPoints: %d"), PathPoints.Num());
		return;
	}

	bIsAutoMoving = true;
	SetComponentTickEnabled(true);

	// 몬스터를 NavMesh Obstacle로 피하기 위해 RVO Avoidance를 활성화합니다.
	if (UCharacterMovementComponent* MovComp = OwnerChar->GetCharacterMovement())
	{
		MovComp->bUseRVOAvoidance = true;
	}

	// 1초마다 경로를 재계산해 장애물 변화를 반영합니다.
	GetWorld()->GetTimerManager().SetTimer(
		RecalcTimerHandle,
		this,
		&UAutoMoveComponent::RecalcPath,
		1.0f,
		true
	);
}

// 자동이동을 중단하고 데칼·타이머·상태를 모두 초기화합니다.
void UAutoMoveComponent::StopAutoMove()
{
	if (!bIsAutoMoving) return;

	bIsAutoMoving = false;
	SetComponentTickEnabled(false);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RecalcTimerHandle);
	}

	ClearPathDecals();
	PathPoints.Empty();
	CurrentPathIndex      = 0;
	StuckTimer            = 0.0f;
	StuckCheckAccumulator = 0.0f;

	// RVO Avoidance를 원래 상태로 되돌립니다.
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar)
	{
		if (UCharacterMovementComponent* MovComp = OwnerChar->GetCharacterMovement())
		{
			MovComp->bUseRVOAvoidance = false;
		}
	}
}

void UAutoMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsAutoMoving) return;

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) { StopAutoMove(); return; }

	FVector CurrentPos = OwnerChar->GetActorLocation();

	// ── 제자리 감지 ──────────────────────────────────────────────────
	// 1초마다 이동 거리를 스냅샷으로 비교합니다.
	StuckCheckAccumulator += DeltaTime;
	if (StuckCheckAccumulator >= 1.0f)
	{
		StuckCheckAccumulator = 0.0f;
		float Moved = FVector::Dist2D(CurrentPos, LastStuckCheckPos);
		LastStuckCheckPos = CurrentPos;

		if (Moved < StuckDistanceThreshold)
		{
			StuckTimer += 1.0f;
			if (StuckTimer >= StuckMaxTime)
			{
				StopAutoMove();
				return;
			}
		}
		else
		{
			// 충분히 이동했으면 제자리 타이머를 초기화합니다.
			StuckTimer = 0.0f;
		}
	}

	// ── 웨이포인트 통과 판정 ─────────────────────────────────────────
	while (CurrentPathIndex < PathPoints.Num())
	{
		if (FVector::Dist2D(CurrentPos, PathPoints[CurrentPathIndex]) <= WaypointAcceptanceRadius)
		{
			CurrentPathIndex++;
		}
		else
		{
			break;
		}
	}

	// 모든 웨이포인트를 통과하면 목적지에 도착한 것으로 간주합니다.
	if (CurrentPathIndex >= PathPoints.Num())
	{
		StopAutoMove();
		return;
	}

	// ── 이동 입력 ────────────────────────────────────────────────────
	FVector Dir = (PathPoints[CurrentPathIndex] - CurrentPos).GetSafeNormal2D();
	OwnerChar->AddMovementInput(Dir, 1.0f);
}

// NavMesh로 경로를 재계산하고 데칼을 갱신합니다.
void UAutoMoveComponent::RecalcPath()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;

	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(
		GetWorld(),
		OwnerChar->GetActorLocation(),
		Destination,
		OwnerChar
	);

	if (!NavPath || NavPath->PathPoints.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 경로 없음. PathPoints: %d / 플레이어: %s / 목적지: %s"),
			NavPath ? NavPath->PathPoints.Num() : -1,
			*OwnerChar->GetActorLocation().ToString(),
			*Destination.ToString());
		if (bIsAutoMoving) StopAutoMove();
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 경로 계산 성공. PathPoints: %d"), NavPath->PathPoints.Num());

	PathPoints       = NavPath->PathPoints;
	CurrentPathIndex = 0;

	// 이미 지나친 웨이포인트를 건너뜁니다.
	FVector CurrentPos = OwnerChar->GetActorLocation();
	while (CurrentPathIndex < PathPoints.Num() - 1)
	{
		if (FVector::Dist2D(CurrentPos, PathPoints[CurrentPathIndex]) <= WaypointAcceptanceRadius)
		{
			CurrentPathIndex++;
		}
		else
		{
			break;
		}
	}

	SpawnPathDecals();
}

// 경로 세그먼트를 따라 DecalSpacing 간격으로 데칼을 스폰합니다.
void UAutoMoveComponent::SpawnPathDecals()
{
	ClearPathDecals();

	if (!PathDecalMaterial) return;
	if (PathPoints.Num() < 2) return;

	for (int32 i = 0; i < PathPoints.Num() - 1; ++i)
	{
		FVector Start  = PathPoints[i];
		FVector End    = PathPoints[i + 1];
		float   SegLen = FVector::Dist(Start, End);
		FVector Dir    = (End - Start).GetSafeNormal();

		float Traveled = 0.0f;
		while (Traveled <= SegLen)
		{
			FVector SpawnPos = Start + Dir * Traveled;
			// 클리핑 방지를 위해 지면 위로 살짝 올립니다.
			SpawnPos.Z += 20.0f;

			UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
				GetWorld(),
				PathDecalMaterial,
				FVector(DecalRadius, DecalRadius, DecalRadius),
				SpawnPos,
				FRotator(-90.0f, 0.0f, 0.0f) // 아래를 향해 지면에 투사합니다.
			);

			if (Decal)
			{
				Decal->SetFadeScreenSize(0.0f);
				PathDecals.Add(Decal);
			}

			Traveled += DecalSpacing;
		}
	}
}

// 스폰된 데칼 액터를 모두 제거합니다.
void UAutoMoveComponent::ClearPathDecals()
{
	for (UDecalComponent* Decal : PathDecals)
	{
		if (IsValid(Decal))
		{
			AActor* Owner = Decal->GetOwner();
			if (IsValid(Owner))
			{
				Owner->Destroy();
			}
		}
	}
	PathDecals.Empty();
}
