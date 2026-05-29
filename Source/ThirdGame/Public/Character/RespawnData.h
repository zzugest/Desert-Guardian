#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RespawnData.generated.h"

// 레벨별 부활 위치를 정의하는 DataTable 행 구조체입니다.
// Row 키는 임의의 식별자로 지정합니다.
USTRUCT(BlueprintType)
struct FRespawnData : public FTableRowBase
{
    GENERATED_BODY()

    // 부활 위치가 적용될 레벨 이름입니다. NAME_None이면 마을(Persistent Level)을 의미합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
    FName LevelName = NAME_None;

    // 부활 시 캐릭터가 스폰될 월드 좌표입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
    FVector RespawnLocation = FVector::ZeroVector;

    // 부활 시 캐릭터가 바라볼 회전값입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
    FRotator RespawnRotation = FRotator::ZeroRotator;
};
