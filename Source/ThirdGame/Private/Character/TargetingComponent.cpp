// =========================================================================================
// TargetingComponent.cpp
//
// [파일 역할]
// 플레이어(카메라) 시점을 기준으로 화면 중앙에 가장 가까운 적을 자동으로 스캔하여
// 타겟팅하는 컴포넌트입니다.
// 거리, 시야각(내적), 그리고 시야 차단 여부 확인(Line Trace)을 필터로 사용하여
// 최적 타겟을 선정합니다.
//
// [타겟 선정 기준]
// 1. 컷신 중이면 즉시 비활성화 (State.Cutscene 태그 확인)
// 2. Overlap Sphere로 TraceDistance 반경 내 액터를 물리 엔진에서 직접 수집
// 3. AEnemy로 캐스트 가능하고 살아있는 적만 필터링
// 4. 카메라 전방 내적값 0.85 이상 (시야각 필터)
// 5. 카메라~적 사이 LineTrace로 벽 차단 여부 확인
// 6. 위 조건을 통과한 것 중 내적값이 가장 높은 적을 타겟으로 선정
// =========================================================================================

#include "TargetingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Enemy.h"
#include "Character/MyCharacter.h"
#include "Engine/OverlapResult.h"

// 컴포넌트 생성 시 틱 활성화 및 초기 타겟을 nullptr로 초기화합니다.
UTargetingComponent::UTargetingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentTarget = nullptr;
}

// 매 프레임마다 FindTarget을 호출하여 화면 중심에 가장 가까운 적을 갱신합니다.
void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    FindTarget();
}

// 카메라 시점 기준으로 화면 중심에 가장 가까운 살아있는 적(Enemy) 액터를 찾아냅니다.
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

    // 카메라 위치와 방향을 구하기 위해 플레이어 컨트롤러가 필요합니다.
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector CameraForward = CameraRotation.Vector();

    // Overlap Sphere로 TraceDistance 반경 내 액터를 물리 엔진에서 직접 수집합니다.
    // 전역 목록 없이 거리 필터를 물리 엔진이 처리하므로 별도 거리 계산이 불필요합니다.
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceDistance);
    FCollisionQueryParams SphereQueryParams;
    SphereQueryParams.AddIgnoredActor(GetOwner());

    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        CameraLocation,
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        SphereQueryParams
    );

    AActor* BestTarget = nullptr;
    float HighestDotProduct = -1.0f;

    for (FOverlapResult& Overlap : Overlaps)
    {
        AEnemy* Enemy = Cast<AEnemy>(Overlap.GetActor());
        if (!Enemy) continue;

        if (Enemy->IsHidden()) continue;
        if (Enemy->bIsDead || !Enemy->bIsTargetable) continue;

        // 카메라 전방 벡터와 적 방향 벡터의 내적으로 시야각을 계산합니다.
        FVector DirectionToEnemy = (Enemy->GetActorLocation() - CameraLocation).GetSafeNormal();
        float DotProduct = FVector::DotProduct(CameraForward, DirectionToEnemy);

        // 시야각 임계값(0.85) 미만이면 화면 가장자리에 있으므로 제외합니다.
        if (DotProduct <= 0.85f) continue;

        // 이미 발견된 타겟보다 화면 중심에 더 가깝지 않으면 건너뜁니다.
        if (DotProduct <= HighestDotProduct) continue;

        // 카메라~적 사이 벽 차단 여부를 LineTrace로 확인합니다.
        FHitResult HitResult;
        FCollisionQueryParams LineQueryParams;
        LineQueryParams.AddIgnoredActor(GetOwner());
        LineQueryParams.AddIgnoredActor(Enemy);

        bool bHitWall = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            CameraLocation,
            Enemy->GetActorLocation(),
            ECC_Visibility,
            LineQueryParams
        );

        // 시야에 벽이나 장애물이 없는 경우만 최적 타겟으로 갱신합니다.
        if (!bHitWall)
        {
            HighestDotProduct = DotProduct;
            BestTarget = Enemy;
        }
    }

    // 찾아낸 최적 타겟(BestTarget)이 이전과 다른 경우에만 마커(시각 피드백) 처리합니다.
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
