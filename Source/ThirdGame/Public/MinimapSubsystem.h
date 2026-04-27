// =========================================================================================
// MinimapSubsystem.h
//
// [파일 역할]
// 미니맵에 표시할 모든 액터(적, NPC, 포탈)를 통합 관리하는 서브시스템입니다.
// 각 액터가 BeginPlay/EndPlay 시점에 직접 등록·해제하며,
// MinimapWidget은 이 목록만 참조해 점을 그립니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MinimapSubsystem.generated.h"

// 미니맵에 표시되는 마커 종류 (색상 결정에 사용)
UENUM(BlueprintType)
enum class EMinimapMarkerType : uint8
{
    Enemy  UMETA(DisplayName = "Enemy"),   // 빨간 점
    NPC    UMETA(DisplayName = "NPC"),     // 노란 점
    Portal UMETA(DisplayName = "Portal")   // 파란 점
};

// 미니맵에 표시할 액터와 타입을 묶는 구조체입니다.
USTRUCT()
struct FMinimapMarker
{
    GENERATED_BODY()

    UPROPERTY()
    AActor* Actor = nullptr;

    EMinimapMarkerType Type = EMinimapMarkerType::Enemy;
};

UCLASS()
class THIRDGAME_API UMinimapSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // 액터를 미니맵 마커로 등록합니다. 중복 등록은 자동으로 무시됩니다.
    void RegisterMarker(AActor* Actor, EMinimapMarkerType Type);

    // 액터를 미니맵 마커 목록에서 제거합니다.
    void UnregisterMarker(AActor* Actor);

    // MinimapWidget이 NativePaint에서 참조하는 마커 목록을 반환합니다.
    const TArray<FMinimapMarker>& GetMarkers() const { return ActiveMarkers; }

private:
    UPROPERTY()
    TArray<FMinimapMarker> ActiveMarkers;
};
