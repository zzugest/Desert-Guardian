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
// 2. AllActiveEnemies 목록 순회 (GetAllActorsOfClass 대신 정적 목록 참조)
// 3. 거리 3000 유닛 이내
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
            // 기존 타겟이 있으면 마커를 제거하고 초기화합니다.
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

    // GetAllActorsOfClass() 대신 Enemy가 스폰/사망 시 자체 관리하는 목록을 직접 참조합니다.
    AActor* BestTarget = nullptr;
    float HighestDotProduct = -1.0f;
    float MaxTargetRange = 3000.0f;

    for (TWeakObjectPtr<AEnemy>& WeakEnemy : AEnemy::AllActiveEnemies)
    {
        // GC에 의해 이미 수거된 객체는 안전하게 건너뜁니다.
        if (!WeakEnemy.IsValid()) continue;
        AEnemy* Enemy = WeakEnemy.Get();

        if (Enemy->IsHidden()) continue;

        if (Enemy->bIsDead || !Enemy->bIsTargetable) continue;

        FVector DirectionToEnemy = (Enemy->GetActorLocation() - CameraLocation);

        // Size() 대신 SizeSquared()로 sqrt 연산 없이 거리를 비교합니다.
        if (DirectionToEnemy.SizeSquared() > MaxTargetRange * MaxTargetRange) continue;

        DirectionToEnemy.Normalize();

        // 카메라 전방 벡터와 적 방향 벡터의 내적으로 시야각을 계산합니다.
        float DotProduct = FVector::DotProduct(CameraForward, DirectionToEnemy);

        // 시야각 임계값(0.85) 미만이면 화면 가장자리에 있으므로 제외합니다.
        if (DotProduct <= 0.85f) continue;

        // 이미 발견된 타겟보다 화면 중심에 더 가깝지 않으면 건너뜁니다.
        if (DotProduct <= HighestDotProduct) continue;

        // 카메라 사이나 시야가 벽 뒤에 가려져 있는지 판별하기 위해 선형체 벽 검사합니다.
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        QueryParams.AddIgnoredActor(Enemy);

        bool bHitWall = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            CameraLocation,
            Enemy->GetActorLocation(),
            ECC_Visibility,
            QueryParams
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
