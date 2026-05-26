// =========================================================================================
// AutoMoveTargetData.h
//
// [파일 역할]
// 퀘스트 자동이동 목적지 데이터를 정의하는 DataTable 행 구조체입니다.
// Row 키 = 퀘스트 RowName (예: Quest_001)
// - HuntTargetLocation : 사냥 단계에서 이동할 위치 (에디터에서 직접 입력)
// - CompletionNPCTag   : 완료 보고 단계에서 찾을 NPC의 액터 태그
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AutoMoveTargetData.generated.h"

USTRUCT(BlueprintType)
struct THIRDGAME_API FAutoMoveTargetData : public FTableRowBase
{
    GENERATED_BODY()

    // 사냥 단계 목적지 위치 (에디터에서 MonsterSpawner 좌표를 직접 입력)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    FVector HuntTargetLocation = FVector::ZeroVector;

    // 완료 보고 단계에서 찾을 NPC의 액터 태그 (레벨에서 태그로 검색)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Completion")
    FName CompletionNPCTag = NAME_None;
};
