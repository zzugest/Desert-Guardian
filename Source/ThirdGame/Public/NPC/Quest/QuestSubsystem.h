// =========================================================================================
// QuestSubsystem.h
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestData.h"
#include "QuestSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnQuestObjectiveUpdated, EQuestTaskType, TaskType, FName, TargetID, int32, Amount);

UCLASS()
class THIRDGAME_API UQuestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // 퀘스트 목표 달성 시 QuestComponent/UI 에 변경을 알리는 이벤트 브로드캐스터
    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnQuestObjectiveUpdated OnQuestObjectiveUpdated;

    // 특정 행동(적 처치, 아이템 획득 등) 발생 시 호출되어 목표 달성 여부를 시스템에 알리는 이벤트를 발생시킵니다.
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateQuestObjective(EQuestTaskType TaskType, FName TargetID, int32 Amount);

    // ===== 레벨 이동 시에도 유지되는 퀘스트 상태 데이터 =====

    // 현재 진행 중인 퀘스트 목록 (GameInstanceSubsystem 이므로 레벨 이동 후에도 유지됨)
    UPROPERTY(BlueprintReadWrite, Category = "Quest System")
    TArray<FActiveQuestInfo> ActiveQuests;

    // 완료된 퀘스트 ID 목록 (GameInstanceSubsystem 이므로 레벨 이동 후에도 유지됨)
    UPROPERTY(BlueprintReadWrite, Category = "Quest System")
    TArray<FName> CompletedQuests;

    // 포탈을 통해 레벨 전환 후 자동이동을 재개해야 하는지 여부
    // GameInstanceSubsystem이므로 레벨 이동 후에도 값이 유지됩니다.
    bool bPendingInterLevelAutoMove = false;
};
