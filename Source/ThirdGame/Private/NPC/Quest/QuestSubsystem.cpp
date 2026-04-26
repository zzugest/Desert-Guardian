// =========================================================================================
// QuestSubsystem.cpp
//
// [파일 역할]
// 퀘스트 진행 상태(ActiveQuests · CompletedQuests)를 레벨 전환에도 유지하는
// GameInstance 서브시스템입니다.
// 몬스터 처치·아이템 사용·골드 수집 등 게임 이벤트가 발생하면
// UpdateQuestObjective를 호출해 OnQuestObjectiveUpdated 델리게이트를 브로드캐스트합니다.
// QuestComponent가 이 델리게이트를 구독하여 진행도를 업데이트합니다.
// =========================================================================================

#include "NPC/Quest/QuestSubsystem.h"

// 퀘스트 목표 이벤트를 브로드캐스트합니다. 실제 진행도 계산은 구독자(QuestComponent)가 담당합니다.
void UQuestSubsystem::UpdateQuestObjective(EQuestTaskType TaskType, FName TargetID, int32 Amount)
{
    OnQuestObjectiveUpdated.Broadcast(TaskType, TargetID, Amount);
}
