#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PortalData.generated.h"

UENUM(BlueprintType)
enum class EPortalType : uint8
{
	LevelTransition     UMETA(DisplayName = "Level Transition"),
	SameLevelTeleport   UMETA(DisplayName = "Same Level Teleport")
};

USTRUCT(BlueprintType)
struct FPortalData : public FTableRowBase
{
	GENERATED_BODY()

	// 포탈 타입: 레벨 전환 or 같은 레벨 내 텔레포트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Data")
	EPortalType PortalType = EPortalType::LevelTransition;

	// 1. 시스템이 사용할 목적지 레벨 이름 (LevelTransition 전용, 예: "Level_Dungeon_01")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Data")
	FName TargetLevelName = TEXT("None");

	// 2. UI에 표시할 한국어 맵 이름 (예: "어둠의 숲 던전 1층")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Data")
	FString LevelKoreaName = TEXT("None");

	// 3. SameLevelTeleport 전용: 목적지 액터의 Actor Tag (레벨에 배치한 TargetPoint 태그)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Data")
	FName TargetActorTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FName PrerequisiteQuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FName PrerequisiteBossRowName;
};
