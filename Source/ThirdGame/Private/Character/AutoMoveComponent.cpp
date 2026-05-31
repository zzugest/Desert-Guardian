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
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "GlobalUI/WarningSubsystem.h"
#include "MyCharacter.h"
#include "CombatComponent.h"
#include "Enemy.h"
#include "DrawDebugHelpers.h"
#include "Portal/MapPortal.h"

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

	// Block.AutoMove 태그가 있으면 자동이동을 시작하지 않습니다.
	AMyCharacter* MyChar = Cast<AMyCharacter>(OwnerChar);
	if (MyChar && MyChar->HasStateTag("Block.AutoMove"))
	{
		UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
		if (GI)
		{
			UWarningSubsystem* WarnSys = GI->GetSubsystem<UWarningSubsystem>();
			if (WarnSys)
			{
				FText WarningText = FText::FromStringTable(
					TEXT("/Game/character/ST_WarningMessages.ST_WarningMessages"),
					TEXT("AutoMove_BlockedByCombat"));
				WarnSys->ShowWarning(WarningText);
			}
		}
		return;
	}

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

	CurrentPathIndex      = 0;
	StuckTimer            = 0.0f;
	StuckCheckAccumulator = 0.0f;
	LastStuckCheckPos     = OwnerChar->GetActorLocation();
	bIsDetouring          = false;
	DetourWaypoint        = FVector::ZeroVector;
	AvoidanceScanAccumulator = 0.0f;

	// 시작 시 이미 겹치는 적은 무시 목록에 등록해 즉시 우회 발동을 방지합니다.
	IgnoredEnemies.Empty();
	TArray<AActor*> NearbyActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), OwnerChar->GetActorLocation(), 300.f,
		ObjTypes, AEnemy::StaticClass(),
		TArray<AActor*>{ OwnerChar }, NearbyActors);
	IgnoredEnemies = NearbyActors;

	// 초기 경로 계산. 경로를 찾지 못하면 시작하지 않습니다.
	RecalcPath();
	if (PathPoints.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 경로 계산 실패 - NavMesh가 목적지까지 생성됐는지 확인. PathPoints: %d"), PathPoints.Num());
		return;
	}

	bIsAutoMoving = true;
	SetComponentTickEnabled(true);
	UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 자동이동 시작 → 목적지: %s"), *Destination.ToString());

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (GI)
	{
		UWarningSubsystem* WarnSys = GI->GetSubsystem<UWarningSubsystem>();
		if (WarnSys)
		{
			FText WarningText = FText::FromStringTable(
				TEXT("/Game/character/ST_WarningMessages.ST_WarningMessages"),
				TEXT("AutoMove_Started"));
			WarnSys->ShowWarning(WarningText);
		}
	}

	// 몬스터를 NavMesh Obstacle로 피하기 위해 RVO Avoidance를 활성화합니다.
	if (UCharacterMovementComponent* MovComp = OwnerChar->GetCharacterMovement())
	{
		MovComp->bUseRVOAvoidance = true;
	}

	// SP가 0보다 크면 Sprint로 시작합니다.
	AMyCharacter* MyCharForSprint = Cast<AMyCharacter>(OwnerChar);
	if (MyCharForSprint && MyCharForSprint->CombatComp && MyCharForSprint->CombatComp->CurrentSP > 0.f)
	{
		MyCharForSprint->StartSprint();
		bAutoMoveSprinting = true;
		UE_LOG(LogTemp, Warning, TEXT("[AutoMove] Sprint 시작"));
	}
	else
	{
		bAutoMoveSprinting = false;
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
	CurrentPathIndex         = 0;
	StuckTimer               = 0.0f;
	StuckCheckAccumulator    = 0.0f;
	bIsDetouring             = false;
	DetourWaypoint           = FVector::ZeroVector;
	AvoidanceScanAccumulator = 0.0f;
	IgnoredEnemies.Empty();

	// RVO Avoidance를 원래 상태로 되돌립니다.
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar)
	{
		if (UCharacterMovementComponent* MovComp = OwnerChar->GetCharacterMovement())
		{
			MovComp->bUseRVOAvoidance = false;
		}

		// 자동이동 중 Sprint 중이었다면 해제합니다.
		if (bAutoMoveSprinting)
		{
			AMyCharacter* MyChar = Cast<AMyCharacter>(OwnerChar);
			if (MyChar) MyChar->StopSprint();
			bAutoMoveSprinting = false;
		}
	}
}

// 플레이어 입력으로 자동이동을 취소하고 취소 경고 메시지를 화면에 표시합니다.
void UAutoMoveComponent::CancelAutoMove()
{
	if (!bIsAutoMoving) return;

	UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 플레이어 입력으로 취소"));
	StopAutoMove();

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (GI)
	{
		UWarningSubsystem* WarnSys = GI->GetSubsystem<UWarningSubsystem>();
		if (WarnSys)
		{
			FText WarningText = FText::FromStringTable(
				TEXT("/Game/character/ST_WarningMessages.ST_WarningMessages"),
				TEXT("AutoMove_Cancelled"));
			WarnSys->ShowWarning(WarningText);
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
			UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 제자리 감지 (StuckTimer: %.0f / %.0f초)"), StuckTimer, StuckMaxTime);
			if (StuckTimer >= StuckMaxTime)
			{
				UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 제자리 한계 → 자동이동 중단"));
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

	// ── 전방 적 탐지 스캔 (목적지 근처에서는 비활성화) ──────────────
	if (FVector::Dist2D(CurrentPos, Destination) > ArrivalAvoidanceDisableRadius)
	{
		AvoidanceScanAccumulator += DeltaTime;
		if (AvoidanceScanAccumulator >= AvoidanceScanInterval)
		{
			AvoidanceScanAccumulator = 0.0f;
			CheckAndDetour();
		}
	}

	// ── 우회 경유지 도달 체크 ─────────────────────────────────────────
	if (bIsDetouring && FVector::Dist2D(CurrentPos, DetourWaypoint) <= WaypointAcceptanceRadius)
	{
		bIsDetouring = false;
		UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 우회 완료 → 경로 재탐색"));
		RecalcPath();
		return;
	}

	// ── 다음 목표 결정 (우회 중이면 경유지, 아니면 일반 웨이포인트) ──
	FVector NextTarget;
	if (bIsDetouring)
	{
		NextTarget = DetourWaypoint;
	}
	else
	{
		// 웨이포인트 통과 판정
		while (CurrentPathIndex < PathPoints.Num())
		{
			if (FVector::Dist2D(CurrentPos, PathPoints[CurrentPathIndex]) <= WaypointAcceptanceRadius)
				CurrentPathIndex++;
			else
				break;
		}

		if (CurrentPathIndex >= PathPoints.Num())
		{
			// 목적지가 포탈이면 확인창 없이 자동으로 통과합니다.
			if (PendingPortal)
			{
				UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 포탈 도착 → 자동 통과"));
				AMyCharacter* MyChar = Cast<AMyCharacter>(OwnerChar);
				if (MyChar)
				{
					AMapPortal* Portal = PendingPortal;
					PendingPortal = nullptr;
					StopAutoMove();
					Portal->TravelThroughPortalDirect(MyChar);
				}
				return;
			}

			UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 목적지 도착 → 자동이동 완료"));

			UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
			if (GI)
			{
				UWarningSubsystem* WarnSys = GI->GetSubsystem<UWarningSubsystem>();
				if (WarnSys)
				{
					FText WarningText = FText::FromStringTable(
						TEXT("/Game/character/ST_WarningMessages.ST_WarningMessages"),
						TEXT("AutoMove_Arrived"));
					WarnSys->ShowWarning(WarningText);
				}
			}

			StopAutoMove();
			return;
		}

		NextTarget = PathPoints[CurrentPathIndex];
	}

	// ── SP 기반 Sprint 제어 ──────────────────────────────────────────
	AMyCharacter* MyChar = Cast<AMyCharacter>(OwnerChar);
	if (MyChar && MyChar->CombatComp)
	{
		const float CurrentSP = MyChar->CombatComp->CurrentSP;
		const float MaxSP     = MyChar->CombatComp->MaxSP;

		if (bAutoMoveSprinting && CurrentSP <= 0.f)
		{
			// SP 소진 → bAutoMoveSprinting만 해제합니다.
			// 실제 Sprint 중단(MaxWalkSpeed 복구·bIsSprinting 해제)은
			// 서버 Tick의 MulticastForceStopSprint가 모든 클라이언트에 처리하므로 중복 호출하지 않습니다.
			bAutoMoveSprinting = false;
			UE_LOG(LogTemp, Warning, TEXT("[AutoMove] SP 소진 → Sprint 중단 (MulticastForceStopSprint 대기)"));
		}
		else if (!bAutoMoveSprinting && CurrentSP >= MaxSP)
		{
			// SP 완전 회복 → Sprint 재개
			MyChar->StartSprint();
			bAutoMoveSprinting = true;
			UE_LOG(LogTemp, Warning, TEXT("[AutoMove] SP 회복 → Sprint 재개"));
		}
	}

	// ── 이동 입력 ────────────────────────────────────────────────────
	FVector Dir = (NextTarget - CurrentPos).GetSafeNormal2D();
	OwnerChar->AddMovementInput(Dir, 1.0f);

}

// 전방 적을 탐지해 경로가 막혀있으면 우회 경유지를 설정합니다.
void UAutoMoveComponent::CheckAndDetour()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar || !bIsAutoMoving) return;

	FVector CurrentPos = OwnerChar->GetActorLocation();

	// 현재 향하고 있는 목표 방향 계산
	FVector CurrentTarget = bIsDetouring ? DetourWaypoint
		: (CurrentPathIndex < PathPoints.Num() ? PathPoints[CurrentPathIndex] : Destination);

	FVector MoveDir = (CurrentTarget - CurrentPos).GetSafeNormal2D();
	if (MoveDir.IsNearlyZero()) return;

	// 전방 DetourScanRange 범위 내 AEnemy 탐색
	TArray<AActor*> OverlapActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), CurrentPos, DetourScanRange,
		ObjTypes, AEnemy::StaticClass(),
		TArray<AActor*>{ OwnerChar }, OverlapActors);

	int32 FrontEnemyCount = 0;

	for (AActor* Actor : OverlapActors)
	{
		AEnemy* Enemy = Cast<AEnemy>(Actor);
		if (!Enemy) continue;
		if (IgnoredEnemies.Contains(Enemy)) continue;

		// 경로 차단 여부로만 판단하므로 전방 dot 조건은 불필요해 주석 처리합니다.
		// FVector ToEnemy = (Enemy->GetActorLocation() - CurrentPos).GetSafeNormal2D();
		// if (FVector::DotProduct(MoveDir, ToEnemy) < 0.3f) continue;

		FrontEnemyCount++;
		float AvoidRadius = Enemy->GetCapsuleComponent()->GetScaledCapsuleRadius() * EnemyAvoidanceMultiplier;

		// 현재 경로(현재위치~목표)와 적 사이의 최소 거리가 회피 반경 이내인지 확인합니다.
		FVector ClosestPt = FMath::ClosestPointOnSegment(Enemy->GetActorLocation(), CurrentPos, CurrentTarget);
		float DistToPath  = FVector::Dist2D(ClosestPt, Enemy->GetActorLocation());

		if (DistToPath < AvoidRadius)
		{
			// 경로를 막는 적은 빨간색으로 표시합니다.
			DrawDebugSphere(GetWorld(), Enemy->GetActorLocation(), AvoidRadius, 16, FColor::Red, false, AvoidanceScanInterval, 0, 2.f);

			FVector OutPoint;
			if (ComputeDetourPoint(Enemy->GetActorLocation(), AvoidRadius, OutPoint))
			{
				DetourWaypoint = OutPoint;
				bIsDetouring   = true;
				UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 적 감지 → 우회 경유지: %s"), *OutPoint.ToString());
				return; // 가장 가까운 적 하나만 처리하고 다음 스캔에서 체이닝합니다.
			}
		}
	}

	if (FrontEnemyCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 전방 적 %d마리 탐지 (경로 교차 없음)"), FrontEnemyCount);
	}
}

// 적 위치를 기준으로 좌/우 경유지를 계산하고 NavMesh에 투영합니다.
bool UAutoMoveComponent::ComputeDetourPoint(FVector EnemyPos, float AvoidRadius, FVector& OutPoint)
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return false;

	FVector CurrentPos = OwnerChar->GetActorLocation();
	FVector MoveDir    = (Destination - CurrentPos).GetSafeNormal2D();
	if (MoveDir.IsNearlyZero()) return false;

	const float Buffer = 100.f;
	FVector PerpLeft   = FVector(-MoveDir.Y,  MoveDir.X, 0.f);
	FVector PerpRight  = FVector( MoveDir.Y, -MoveDir.X, 0.f);

	// 적 위치 기준으로 좌/우 후보 경유지를 구합니다 (Z는 플레이어 높이 사용).
	FVector LeftPoint  = FVector(EnemyPos.X, EnemyPos.Y, CurrentPos.Z) + PerpLeft  * (AvoidRadius + Buffer);
	FVector RightPoint = FVector(EnemyPos.X, EnemyPos.Y, CurrentPos.Z) + PerpRight * (AvoidRadius + Buffer);

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return false;

	FNavLocation NavLeft, NavRight;
	bool bLeftOk  = NavSys->ProjectPointToNavigation(LeftPoint,  NavLeft,  FVector(150.f, 150.f, 300.f));
	bool bRightOk = NavSys->ProjectPointToNavigation(RightPoint, NavRight, FVector(150.f, 150.f, 300.f));

	if (!bLeftOk && !bRightOk)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 우회 경로 없음 (좌/우 모두 NavMesh 투영 실패)"));
		return false;
	}

	if (bLeftOk && bRightOk)
	{
		// Destination까지 거리가 짧은 쪽을 선택합니다.
		float DistLeft  = FVector::Dist2D(NavLeft.Location,  Destination);
		float DistRight = FVector::Dist2D(NavRight.Location, Destination);
		OutPoint = (DistLeft < DistRight) ? NavLeft.Location : NavRight.Location;
	}
	else
	{
		OutPoint = bLeftOk ? NavLeft.Location : NavRight.Location;
	}

	return true;
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
