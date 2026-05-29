// =========================================================================================
// QuestComponent.h
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestData.h"
#include "QuestComponent.generated.h"

// 퀘스트 상태가 변경되었을 때 UI 화면 갱신을 요청하기 위한 브로드캐스터
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestUIUpdated);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THIRDGAME_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestComponent();

protected:
	// 컴포넌트 초기화 시 퀘스트 이벤트 브로드캐스터 바인딩을 설정합니다.
	virtual void BeginPlay() override;

	// 컴포넌트 파괴 시 델리게이트 바인딩을 해제합니다. (중복 등록 방지)
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// QuestSubsystem을 가져오는 헬퍼 함수
	class UQuestSubsystem* GetQuestSubsystem() const;

	// 특정 행동(적 처치, 아이템 획득 등) 발생 시 진행 목표치를 업데이트하는 퀘스트 시스템 콜백입니다.
	UFUNCTION()
	void HandleQuestObjectiveUpdated(EQuestTaskType TaskType, FName TargetID, int32 Amount);

public:
	// ============================
	// 데이터: QuestDataTable
	// ============================

	// 퀘스트 이름, 목표, 보상 등이 담긴 데이터 테이블입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	UDataTable* QuestDataTable;

	// ============================
	// UI 알림(브로드캐스터)
	// ============================

	// 퀘스트 상태 변경 시 인벤토리나 퀘스트 로그 UI를 다시 그리도록 요청하는 이벤트 인스턴스
	UPROPERTY(BlueprintAssignable, Category = "Quest UI")
	FOnQuestUIUpdated OnQuestUIUpdated;

	// 현재 진행 중인 퀘스트의 목표 현황을 UI 텍스트 형태로 포매팅하여 반환합니다.
	UFUNCTION(BlueprintPure, Category = "Quest UI")
	FString GetQuestLogText();

	// ============================
	// 퀘스트 수락 / 완료 기능
	// ============================

	// 데이터 테이블에서 데이터를 가져와 플레이어의 활성 퀘스트 목록에 새 퀘스트를 추가합니다.
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void AcceptQuest(FName NewQuestID);

	// 퀘스트 완료 처리를 수행하고 해당 퀘스트를 완료 목록으로 이동시킵니다.
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteQuest(FName QuestID);

	// 해당 퀘스트의 선행 조건을 만족하여 수락 가능한지 판단합니다.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool CanAcceptQuest(FName QuestID);

	// 해당 퀘스트가 현재 활성(Active) 목록에 포함되어 있는지 확인합니다.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool IsQuestActive(FName QuestID);

	// 해당 퀘스트의 목표를 모두 달성하여 NPC에게 납기(Turn in) 가능한 상태인지 확인합니다.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool IsQuestReadyToComplete(FName QuestID);

	// 해당 퀘스트를 이미 완료한 클리어 상태인지 확인합니다.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool IsQuestCompleted(FName QuestID);

	// 현재 퀘스트 단계에 따라 자동이동 목적지를 결정하고 이동을 시작합니다.
	// 사냥 단계 → AutoMoveTargetTable의 HuntTargetLocation으로 이동
	// 완료 보고 단계 → AutoMoveTargetTable의 CompletionNPCTag로 NPC를 찾아 이동
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void StartAutoMoveToHuntTarget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* ItemDataTable;

	// 퀘스트 자동이동 목적지 테이블 (Row 키 = 퀘스트 RowName)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	UDataTable* AutoMoveTargetTable;

	// 레벨 간 연결 관계를 정의하는 테이블 (BFS 경로 탐색에 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	UDataTable* LevelGraphTable;
};
