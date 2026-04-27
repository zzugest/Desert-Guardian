// =========================================================================================
// MinimapSubsystem.cpp
//
// [파일 역할]
// 미니맵 마커 목록을 관리하는 서브시스템입니다.
// 적은 MonsterSpawner의 4가지 시점에서, NPC·포탈은 BeginPlay/EndPlay에서 등록·해제합니다.
// =========================================================================================

#include "MinimapSubsystem.h"

// 중복 등록을 방지하고 액터를 마커 목록에 추가합니다.
void UMinimapSubsystem::RegisterMarker(AActor* Actor, EMinimapMarkerType Type)
{
    if (!Actor) return;

    // 이미 등록된 액터는 건너뜁니다.
    for (const FMinimapMarker& Marker : ActiveMarkers)
    {
        if (Marker.Actor == Actor) return;
    }

    FMinimapMarker NewMarker;
    NewMarker.Actor = Actor;
    NewMarker.Type  = Type;
    ActiveMarkers.Add(NewMarker);
}

// 액터를 목록에서 제거하고, GC로 수거된 무효 항목도 함께 정리합니다.
void UMinimapSubsystem::UnregisterMarker(AActor* Actor)
{
    if (!Actor) return;

    ActiveMarkers.RemoveAll([Actor](const FMinimapMarker& Marker)
    {
        return Marker.Actor == Actor || !IsValid(Marker.Actor);
    });
}
