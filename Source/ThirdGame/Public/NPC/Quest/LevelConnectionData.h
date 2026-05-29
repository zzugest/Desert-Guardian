#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LevelConnectionData.generated.h"

// =========================================================================================
// LevelConnectionData.h
//
// [파일 역할]
// 레벨 간 연결 관계를 정의하는 DataTable 행 구조체입니다.
// Row 키는 임의의 식별자(예: Edge_01)로 지정합니다.
// BFS 경로 탐색 시 이 테이블을 그래프의 간선(Edge)으로 사용합니다.
// =========================================================================================

USTRUCT(BlueprintType)
struct THIRDGAME_API FLevelConnectionData : public FTableRowBase
{
    GENERATED_BODY()

    // 출발 레벨 이름 (예: ZoneA). NAME_None이면 마을(Persistent Level)을 의미합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Graph")
    FName FromLevel = NAME_None;

    // 도착 레벨 이름 (예: ZoneB). NAME_None이면 마을(Persistent Level)을 의미합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Graph")
    FName ToLevel = NAME_None;
};
