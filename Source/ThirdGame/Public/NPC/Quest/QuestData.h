// =========================================================================================
// QuestData.h
//
// [역할 요약]
// 데이터테이블(엑셀) 등에 정의할 퀘스트 원본 스펙과 런타임에서 플레이어의 퀘스트/목표 진행 상태를 추적하기 위한 데이터 구조체들을 선언하는 헤더입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "QuestData.generated.h"

// 사냥, 아이템 사용 등 퀘스트의 세부 목표 종류를 구분하기 위한 분류용 데이터입니다.
UENUM(BlueprintType)
enum class EQuestTaskType : uint8
{
    Hunt        UMETA(DisplayName = "Hunt Monster"),
    UseItem     UMETA(DisplayName = "Use Item"),
    CollectGold UMETA(DisplayName = "Collect Gold")
};

// ==========================================================
// 퀘스트 목표 데이터 구조체
// ==========================================================
// 기획 데이터 테이블에 기입될 단일 단위의 퀘스트 목표 요구 사항입니다.
USTRUCT(BlueprintType)
struct FQuestTaskData
{
    GENERATED_BODY()

public:
    // 달성해야 할 목표의 종류입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Task")
    EQuestTaskType TaskType = EQuestTaskType::Hunt;

    // 목표 대상의 고유 식별자(ID)입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Task")
    FName TargetID;

    // UI 화면에 출력될 목표의 이름입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Task")
    FString TaskName;

    // 목표 달성에 필요한 총 요구 수량입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Task")
    int32 RequiredAmount = 1;
};



// ==========================================================
// 퀘스트 아이템 보상 구조체
// ==========================================================
USTRUCT(BlueprintType)
struct FQuestItemReward
{
    GENERATED_BODY()

public:
    // 지급할 아이템 데이터 테이블의 행 이름(Row Name)입니다. (예: "Item_RedPotion")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FName ItemRowName;

    // 플레이어에게 지급할 수량입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 Quantity = 1;
};



// ==========================================================
// 퀘스트 원본 데이터 구조체
// ==========================================================
// 데이터 테이블에서 관리될 단일 퀘스트의 전체 명세(목표, 보상, 대사 등) 구조체입니다.
USTRUCT(BlueprintType)
struct FQuestData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // UI에 표시될 퀘스트의 제목입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    // 퀘스트에 대한 상세 설명 및 스토리 텍스트입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    // 해당 퀘스트를 완료하기 위해 달성해야 하는 세부 목표들의 목록입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuestTaskData> Tasks;

    // 퀘스트 완료 시 플레이어에게 지급될 골드 액수입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    int32 RewardGold = 0;

    // 퀘스트 완료 시 플레이어에게 지급될 경험치 량입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    int32 RewardExp = 0;


    // 퀘스트 완료 시 지급될 아이템 보상 목록입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    TArray<FQuestItemReward> RewardItems;


    // 이 퀘스트를 수주하기 위해 미리 완료되어야 하는 선행 퀘스트 목록입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Condition")
    TArray<FName> PrerequisiteQuests;

    // NPC와 상호작용 시 순차적으로 출력될 첫 번째 안내 대사들입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    TArray<FText> StartDialogues;

    // 플레이어가 퀘스트를 수락했을 때 NPC가 출력할 대사입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    TArray<FText> AcceptDialogues;

    // 플레이어가 퀘스트를 거절했을 때 NPC가 출력할 대사입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    TArray<FText> DeclineDialogues;

    // 퀘스트 진행 중(미완료) 상태로 대화할 때 NPC가 출력할 대사입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    FText InProgressDialogue;

    // 퀘스트 목표를 모두 달성한 뒤 완료(보상 획득) 단계에서 NPC가 출력할 대사입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    TArray<FText> CompletedDialogues;
};

// ==========================================================
// 퀘스트 목표 진행도 구조체
// ==========================================================
// 인게임 런타임에서 플레이어의 단일 퀘스트 목표 달성 진행 상황을 실시간으로 기록하는 구조체입니다.
USTRUCT(BlueprintType)
struct FQuestTaskProgress
{
    GENERATED_BODY()

public:
    // 대상 목표의 종류입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    EQuestTaskType TaskType = EQuestTaskType::Hunt;

    // 대상 목표의 고유 식별자(ID)입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    FName TargetID;

    // UI에 출력할 목표의 이름입니다.
    FString TaskName;

    // 현재까지 달성한 누적 진행 수량입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    int32 CurrentAmount = 0;

    // 목표 달성에 필요한 총 요구 수량입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    int32 RequiredAmount = 1;

    // 해당 세부 목표를 완전히 달성했는지 여부를 나타냅니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    bool bIsCompleted = false;
};

// ==========================================================
// 진행 중인 퀘스트 정보 구조체
// ==========================================================
// 플레이어가 수락하여 현재 진행 중인 특정 퀘스트 전체의 진행 상태를 보관하는 구조체입니다.
USTRUCT(BlueprintType)
struct FActiveQuestInfo
{
    GENERATED_BODY()

public:
    // 수주한 퀘스트의 원본 식별 고유 ID입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    FName QuestID;

    // UI에 출력할 퀘스트의 제목입니다.
    FString QuestName;

    // 이 퀘스트에 속한 세부 목표들의 개별 진행 상황 목록입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    TArray<FQuestTaskProgress> TaskProgresses;

    // 모든 세부 목표를 달성하여 즉시 보상을 수령할 수 있는 상태인지 판별합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    bool bIsReadyToComplete = false;
};