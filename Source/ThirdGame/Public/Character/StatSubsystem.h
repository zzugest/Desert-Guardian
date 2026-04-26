// =========================================================================================
// StatSubsystem.h
//
// [역할 요약]
// 맵 이동이나 씬 전환 시 플레이어의 스탯(체력, 마나, SP)이 초기화되지 않도록 임시로 보관하는 전역 서브시스템 헤더입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StatSubsystem.generated.h"

UCLASS()
class THIRDGAME_API UStatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 서브시스템 생성 시 보관용 데이터들을 초기 상태(-1)로 세팅합니다.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	// 씬 전환 시 복원을 위해 임시로 저장해둔 현재 체력 값입니다. (초기값 -1은 미저장 상태를 의미)
	UPROPERTY(BlueprintReadWrite, Category = "Saved Stats")
	float SavedCurrentHP = -1.0f;

	// 씬 전환 시 복원을 위해 임시로 저장해둔 현재 마나 값입니다.
	UPROPERTY(BlueprintReadWrite, Category = "Saved Stats")
	float SavedCurrentMP = -1.0f;

	// 씬 전환 시 복원을 위해 임시로 저장해둔 현재 스태미나 값입니다.
	UPROPERTY(BlueprintReadWrite, Category = "Saved Stats")
	float SavedCurrentSP = -1.0f;

	// 캐릭터의 최대 체력 보존을 위한 저장 값입니다.
	UPROPERTY(BlueprintReadWrite, Category = "Saved Stats")
	float SavedMaxHP = 100.0f;

	// 캐릭터의 최대 마나 보존을 위한 저장 값입니다.
	UPROPERTY(BlueprintReadWrite, Category = "Saved Stats")
	float SavedMaxMP = 100.0f;

	// 캐릭터의 최대 스태미나 보존을 위한 저장 값입니다.
	UPROPERTY(BlueprintReadWrite, Category = "Saved Stats")
	float SavedMaxSP = 100.0f;
};