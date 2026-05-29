#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PortalData.generated.h"

UENUM(BlueprintType)
enum class EPortalType : uint8
{
	LevelTransition     UMETA(DisplayName = "Level Transition"),
	SameLevelTeleport   UMETA(DisplayName = "Same Level Teleport"),
	AnotherSubLevel     UMETA(DisplayName = "Another Sub Level")
};

USTRUCT(BlueprintType)
struct FPortalData : public FTableRowBase
{
	GENERATED_BODY()

	// 포탈 타입: 레벨 전환 or 같은 레벨 내 텔레포트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Data")
	EPortalType PortalType = EPortalType::LevelTransition;

	// 2. UI에 표시할 한국어 맵 이름 (예: "어둠의 숲 던전 1층")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Data")
	FString LevelKoreaName = TEXT("None");

	// 3. SameLevelTeleport / AnotherSubLevel 전용: 목적지 월드 좌표 (DataTable에 직접 입력)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Data")
	FVector TargetLocation = FVector::ZeroVector;

	// 5. SameLevelTeleport 전용: 텔레포트 후 캐릭터가 바라볼 회전값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Data")
	FRotator TargetRotation = FRotator::ZeroRotator;

	// 6. SameLevelTeleport 전용: 텔레포트 전 로드할 서브레벨 이름 (예: "DesertTemple"), 마을(Persistent Level)이면 비워둠
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Data")
	FName TargetSubLevelName = NAME_None;

	// 7. SameLevelTeleport 전용: 텔레포트 후 언로드할 서브레벨 이름 (예: "Showcase"), 마을로 복귀 시 현재 서브레벨 이름 입력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Data")
	FName CurrentSubLevelName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FName PrerequisiteQuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FName PrerequisiteBossRowName;
};
