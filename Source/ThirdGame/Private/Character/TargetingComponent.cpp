// =========================================================================================
// TargetingComponent.cpp
//
// [파일 역할]
// 카메라 전방으로 구체를 스윕(SweepMultiByChannel)하여
// 경로상의 적 중 가장 가까운 적을 자동으로 타겟팅하는 컴포넌트입니다.
//
// [타겟 선정 기준]
// 1. 컷신 중이면 즉시 비활성화 (State.Cutscene 태그 확인)
// 2. 카메라 전방으로 SweepRadius 폭, TraceDistance 길이의 구체 스윕
// 3. AEnemy로 캐스트 가능하고 살아있는 적만 필터링
// 4. 스윕 결과는 거리순 정렬 → 첫 번째 유효한 적이 가장 가까운 적
// 5. 카메라~적 사이 LineTrace로 벽 차단 여부 확인
// =========================================================================================

#include "TargetingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Enemy.h"
#include "Character/MyCharacter.h"

// 컴포넌트 생성 시 틱 활성화 및 초기 타겟을 nullptr로 초기화합니다.
// TickInterval을 0.1초로 설정해 매 프레임 대신 초당 10회만 스캔합니다.
UTargetingComponent::UTargetingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    CurrentTarget = nullptr;
}

// 0.1초 간격으로 FindTarget을 호출하여 화면 중심에 가장 가까운 적을 갱신합니다.
void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    FindTarget();
}

// 카메라 전방 스윕으로 가장 가까운 살아있는 적(Enemy)을 찾아 타겟팅합니다.
void UTargetingComponent::FindTarget()
{
    // 컷신 중에는 타겟팅을 비활성화합니다.
    if (AMyCharacter* Owner = Cast<AMyCharacter>(GetOwner()))
    {
        if (Owner->HasStateTag("State.Cutscene"))
        {
            if (AEnemy* OldEnemy = Cast<AEnemy>(CurrentTarget))
            {
                OldEnemy->SetTargetMarkerVisibility(false);
            }
            CurrentTarget = nullptr;
            return;
        }
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
    FVector CameraForward = CameraRotation.Vector();

    // 카메라 전방으로 구체를 스윕하여 경로상의 적을 거리순으로 수집합니다.
    TArray<FHitResult> HitResults;
    FCollisionQueryParams SweepParams;
    SweepParams.AddIgnoredActor(GetOwner());

    GetWorld()->SweepMultiByChannel(
        HitResults,
        CameraLocation,
        CameraLocation + CameraForward * TraceDistance,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(SweepRadius),
        SweepParams
    );

    AActor* BestTarget = nullptr;

    // 결과가 거리순으로 정렬되므로 조건을 통과하는 첫 번째 적이 가장 가까운 타겟입니다.
    for (FHitResult& Hit : HitResults)
    {
        AEnemy* Enemy = Cast<AEnemy>(Hit.GetActor());
        if (!Enemy) continue;
        if (Enemy->IsHidden() || Enemy->bIsDead || !Enemy->bIsTargetable) continue;

        // 카메라 전방 기준 30도(cos 0.866) 이내의 적만 허용합니다.
        FVector DirectionToEnemy = (Enemy->GetActorLocation() - CameraLocation).GetSafeNormal();
        if (FVector::DotProduct(CameraForward, DirectionToEnemy) < 0.707f) continue;

        // 카메라~적 사이 벽 차단 여부를 LineTrace로 확인합니다.
        FHitResult WallHit;
        FCollisionQueryParams LineParams;
        LineParams.AddIgnoredActor(GetOwner());
        LineParams.AddIgnoredActor(Enemy);

        bool bHitWall = GetWorld()->LineTraceSingleByChannel(
            WallHit,
            CameraLocation,
            Enemy->GetActorLocation(),
            ECC_Visibility,
            LineParams
        );

        if (!bHitWall)
        {
            BestTarget = Enemy;
            break; // 가장 가까운 유효한 적 하나만 선택합니다.
        }
    }

 #if ENABLE_DRAW_DEBUG
     if (bShowDebug)
     {
         FVector EndPoint = CameraLocation + CameraForward * TraceDistance;
         DrawDebugLine(GetWorld(), CameraLocation, EndPoint, FColor::Yellow, false, 0.15f, 0, 2.0f);
         DrawDebugCone(GetWorld(), CameraLocation, CameraForward, TraceDistance,
             FMath::DegreesToRadians(30.0f), FMath::DegreesToRadians(30.0f),
             16, FColor::Green, false, 0.15f);
         DrawDebugSphere(GetWorld(), EndPoint, SweepRadius, 16, FColor::Yellow, false, 0.15f);
         if (BestTarget)
             DrawDebugSphere(GetWorld(), BestTarget->GetActorLocation(), 80.0f, 12, FColor::Red, false, 0.15f);
     }
 #endif

    // 타겟이 변경된 경우에만 마커(시각 피드백)를 갱신합니다.
    if (CurrentTarget != BestTarget)
    {
        if (AEnemy* OldEnemy = Cast<AEnemy>(CurrentTarget))
        {
            OldEnemy->SetTargetMarkerVisibility(false);
        }

        if (AEnemy* NewEnemy = Cast<AEnemy>(BestTarget))
        {
            NewEnemy->SetTargetMarkerVisibility(true);
        }

        CurrentTarget = BestTarget;
    }
}
