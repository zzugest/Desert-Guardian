// =========================================================================================
// QuestComponent.cpp
//
// [파일 역할]
// 플레이어에게 붙어 퀘스트 수락·진행도 추적·완료·보상 지급을 담당하는 컴포넌트입니다.
// 퀘스트 상태(ActiveQuests · CompletedQuests)는 QuestSubsystem에 저장해 레벨 이동 후에도 유지됩니다.
// QuestSubsystem의 OnQuestObjectiveUpdated 델리게이트를 구독해 몬스터 처치·아이템 사용·골드 수집을
// 자동으로 진행도에 반영하고, 모든 태스크 완료 시 bIsReadyToComplete를 true로 설정합니다.
// =========================================================================================

#include "NPC/Quest/QuestComponent.h"
#include "Engine/Engine.h"
#include "NPC/Quest/QuestSubsystem.h"
#include "MoneySubsystem.h"
#include "Inventory/InventorySubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Item/ItemData.h"
#include "MyCharacter.h"
#include "AutoMoveComponent.h"
#include "NPC/Quest/AutoMoveTargetData.h"
#include "NPC/Quest/LevelConnectionData.h"
#include "Portal/MapPortal.h"
#include "Kismet/GameplayStatics.h"

UQuestComponent::UQuestComponent()
{
    // 정적 컴포넌트이므로 Tick을 비활성화합니다.
    PrimaryComponentTick.bCanEverTick = false;
}

// QuestSubsystem을 가져오는 내부 헬퍼 함수입니다.
UQuestSubsystem* UQuestComponent::GetQuestSubsystem() const
{
    UGameInstance* GameInst = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    if (!GameInst) return nullptr;
    return GameInst->GetSubsystem<UQuestSubsystem>();
}

// QuestSubsystem의 목표 업데이트 이벤트를 구독합니다.
void UQuestComponent::BeginPlay()
{
    Super::BeginPlay();

    UQuestSubsystem* QuestSub = GetQuestSubsystem();
    if (!QuestSub) return;

    QuestSub->OnQuestObjectiveUpdated.AddDynamic(this, &UQuestComponent::HandleQuestObjectiveUpdated);
}

// 레벨 이동 등으로 컴포넌트가 파괴될 때 델리게이트 등록을 해제합니다.
// 해제하지 않으면 레벨 재진입 시 BeginPlay에서 중복 등록되어 진행도가 2배로 오르는 버그가 생깁니다.
void UQuestComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    UQuestSubsystem* QuestSub = GetQuestSubsystem();
    if (!QuestSub) return;

    QuestSub->OnQuestObjectiveUpdated.RemoveDynamic(this, &UQuestComponent::HandleQuestObjectiveUpdated);
}

// 새 퀘스트를 수락하고 QuestSubsystem의 ActiveQuests에 추가합니다.
// 이미 진행 중이거나 완료된 퀘스트는 중복 수락을 막습니다.
void UQuestComponent::AcceptQuest(FName NewQuestID)
{
    if (!QuestDataTable) return;

    UQuestSubsystem* QuestSub = GetQuestSubsystem();
    if (!QuestSub) return;

    // 이미 완료된 퀘스트는 재수락하지 않습니다.
    if (QuestSub->CompletedQuests.Contains(NewQuestID)) return;

    // 이미 진행 중인 퀘스트는 중복 수락하지 않습니다.
    for (const FActiveQuestInfo& ActiveQuest : QuestSub->ActiveQuests)
    {
        if (ActiveQuest.QuestID == NewQuestID) return;
    }

    FQuestData* FoundQuest = QuestDataTable->FindRow<FQuestData>(NewQuestID, TEXT("AcceptQuestContext"));
    if (!FoundQuest) return;

    // DataTable의 태스크 목록을 초기 진행도(0)로 변환해 ActiveQuests에 등록합니다.
    FActiveQuestInfo NewActiveQuest;
    NewActiveQuest.QuestID           = NewQuestID;
    NewActiveQuest.bIsReadyToComplete = false;
    NewActiveQuest.QuestName         = FoundQuest->QuestName;

    for (const FQuestTaskData& Task : FoundQuest->Tasks)
    {
        FQuestTaskProgress NewTaskProgress;
        NewTaskProgress.TaskType       = Task.TaskType;
        NewTaskProgress.TargetID       = Task.TargetID;
        NewTaskProgress.RequiredAmount = Task.RequiredAmount;
        NewTaskProgress.TaskName       = Task.TaskName;
        NewTaskProgress.CurrentAmount  = 0;
        NewTaskProgress.bIsCompleted   = false;
        NewActiveQuest.TaskProgresses.Add(NewTaskProgress);
    }

    // QuestSubsystem에 저장하므로 레벨 이동 후에도 데이터가 유지됩니다.
    QuestSub->ActiveQuests.Add(NewActiveQuest);

    OnQuestUIUpdated.Broadcast();
}

// QuestSubsystem이 브로드캐스트한 목표 이벤트를 받아 진행 중인 퀘스트 태스크를 업데이트합니다.
// 모든 태스크가 완료되면 해당 퀘스트를 완료 대기(bIsReadyToComplete = true) 상태로 전환합니다.
void UQuestComponent::HandleQuestObjectiveUpdated(EQuestTaskType TaskType, FName TargetID, int32 Amount)
{
    UQuestSubsystem* QuestSub = GetQuestSubsystem();
    if (!QuestSub) return;

    for (FActiveQuestInfo& ActiveQuest : QuestSub->ActiveQuests)
    {
        // 이미 완료 대기 상태인 퀘스트는 건너뜁니다.
        if (ActiveQuest.bIsReadyToComplete) continue;

        bool bAllTasksCompleted = true;

        for (FQuestTaskProgress& Task : ActiveQuest.TaskProgresses)
        {
            if (Task.bIsCompleted) continue;

            if (Task.TaskType == TaskType && Task.TargetID == TargetID)
            {
                Task.CurrentAmount += Amount;

                if (Task.CurrentAmount >= Task.RequiredAmount)
                {
                    // 요구 수량을 초과하지 않도록 클램프합니다.
                    Task.CurrentAmount = Task.RequiredAmount;
                    Task.bIsCompleted  = true;
                }
            }

            if (!Task.bIsCompleted)
            {
                bAllTasksCompleted = false;
            }
        }

        // 모든 태스크가 끝나면 NPC에서 완료 대화를 할 수 있는 상태로 전환합니다.
        if (bAllTasksCompleted)
        {
            ActiveQuest.bIsReadyToComplete = true;
        }
    }

    OnQuestUIUpdated.Broadcast();
}

// 퀘스트 로그 위젯에 표시할 텍스트를 생성합니다.
// 완료 대기 퀘스트 이름은 노란색 RichText 태그로 감쌉니다.
FString UQuestComponent::GetQuestLogText()
{
    UQuestSubsystem* QuestSub = GetQuestSubsystem();
    if (!QuestSub) return FString();

    FString ResultText;

    for (const FActiveQuestInfo& ActiveQuest : QuestSub->ActiveQuests)
    {
        if (ActiveQuest.bIsReadyToComplete)
        {
            ResultText += FString::Printf(TEXT("<Yellow>[%s]</>\n"), *ActiveQuest.QuestName);
        }
        else
        {
            ResultText += FString::Printf(TEXT("[%s]\n"), *ActiveQuest.QuestName);
        }

        for (const FQuestTaskProgress& Task : ActiveQuest.TaskProgresses)
        {
            if (Task.bIsCompleted)
            {
                // 완료된 태스크도 노란색으로 표시합니다.
                ResultText += FString::Printf(TEXT("  <Yellow>- %s (%d / %d)</>\n"),
                    *Task.TaskName, Task.CurrentAmount, Task.RequiredAmount);
            }
            else
            {
                ResultText += FString::Printf(TEXT("  - %s (%d / %d)\n"),
                    *Task.TaskName, Task.CurrentAmount, Task.RequiredAmount);
            }
        }

        ResultText += TEXT("\n");
    }

    return ResultText;
}

// 완료 대기 상태의 퀘스트를 최종 완료 처리하고 골드·아이템 보상을 지급합니다.
void UQuestComponent::CompleteQuest(FName QuestID)
{
    UQuestSubsystem* QuestSub = GetQuestSubsystem();
    if (!QuestSub) return;

    // ActiveQuests에서 해당 퀘스트 인덱스를 검색합니다.
    int32 FoundIndex = INDEX_NONE;
    for (int32 i = 0; i < QuestSub->ActiveQuests.Num(); ++i)
    {
        if (QuestSub->ActiveQuests[i].QuestID == QuestID)
        {
            FoundIndex = i;
            break;
        }
    }

    if (FoundIndex == INDEX_NONE) return;

    // 완료 대기 상태가 아니면 아직 완료할 수 없습니다.
    if (!QuestSub->ActiveQuests[FoundIndex].bIsReadyToComplete) return;

    if (!QuestDataTable) return;

    FQuestData* QuestData = QuestDataTable->FindRow<FQuestData>(QuestID, TEXT("CompleteQuestContext"));
    if (QuestData)
    {
        // 골드 보상 지급
        if (QuestData->RewardGold > 0)
        {
            UMoneySubsystem* MoneySub = GetWorld()->GetGameInstance()->GetSubsystem<UMoneySubsystem>();
            if (MoneySub)
            {
                MoneySub->AddGold(QuestData->RewardGold);
            }
        }

        // 아이템 보상 지급: 서버 RPC를 통해 서버 권위 인벤토리에 추가합니다.
        if (ItemDataTable)
        {
            AMyCharacter* MyChar = Cast<AMyCharacter>(GetOwner());
            if (MyChar && MyChar->InventoryComp)
            {
                for (const FQuestItemReward& RewardNode : QuestData->RewardItems)
                {
                    FItemData* FoundItem = ItemDataTable->FindRow<FItemData>(RewardNode.ItemRowName, TEXT("QuestRewardLookup"));
                    if (FoundItem)
                    {
                        FItemData ItemToGive = *FoundItem;
                        ItemToGive.Quantity  = RewardNode.Quantity;
                        MyChar->ServerClaimQuestReward(ItemToGive);
                    }
                }
            }
        }
    }

    // QuestSubsystem에 완료 기록을 남기고 활성 목록에서 제거합니다.
    // Subsystem에 저장되므로 레벨 이동 후에도 완료 상태가 유지됩니다.
    QuestSub->CompletedQuests.Add(QuestID);
    QuestSub->ActiveQuests.RemoveAt(FoundIndex);

    OnQuestUIUpdated.Broadcast();
}

// 퀘스트를 새로 수락할 수 있는지 확인합니다.
// 이미 진행 중이거나 완료됐으면 false, 선행 퀘스트가 미완료면 false를 반환합니다.
bool UQuestComponent::CanAcceptQuest(FName QuestID)
{
    if (IsQuestActive(QuestID) || IsQuestCompleted(QuestID)) return false;

    if (!QuestDataTable) return false;

    FQuestData* QuestData = QuestDataTable->FindRow<FQuestData>(QuestID, TEXT("CheckPrerequisites"));
    if (!QuestData) return false;

    UQuestSubsystem* QuestSub = GetQuestSubsystem();
    if (!QuestSub) return false;

    // 선행 퀘스트가 모두 완료된 경우에만 수락 가능합니다.
    for (FName PrereqID : QuestData->PrerequisiteQuests)
    {
        if (!QuestSub->CompletedQuests.Contains(PrereqID)) return false;
    }

    return true;
}

// 해당 퀘스트가 현재 진행 중인지 확인합니다.
bool UQuestComponent::IsQuestActive(FName QuestID)
{
    UQuestSubsystem* QuestSub = GetQuestSubsystem();
    if (!QuestSub) return false;

    for (const FActiveQuestInfo& ActiveQuest : QuestSub->ActiveQuests)
    {
        if (ActiveQuest.QuestID == QuestID) return true;
    }
    return false;
}

// 해당 퀘스트가 완료 대기(모든 태스크 완료) 상태인지 확인합니다.
bool UQuestComponent::IsQuestReadyToComplete(FName QuestID)
{
    UQuestSubsystem* QuestSub = GetQuestSubsystem();
    if (!QuestSub) return false;

    for (const FActiveQuestInfo& ActiveQuest : QuestSub->ActiveQuests)
    {
        if (ActiveQuest.QuestID == QuestID) return ActiveQuest.bIsReadyToComplete;
    }
    return false;
}

// 해당 퀘스트가 최종 완료됐는지 확인합니다.
bool UQuestComponent::IsQuestCompleted(FName QuestID)
{
    UQuestSubsystem* QuestSub = GetQuestSubsystem();
    if (!QuestSub) return false;

    return QuestSub->CompletedQuests.Contains(QuestID);
}

// 현재 퀘스트 단계에 따라 자동이동 목적지를 결정하고 이동을 시작합니다.
// 사냥 단계(미완료 태스크 있음) → AutoMoveTargetTable의 HuntTargetLocation으로 이동
// 완료 보고 단계(bIsReadyToComplete) → AutoMoveTargetTable의 CompletionNPCTag로 NPC를 찾아 이동
void UQuestComponent::StartAutoMoveToHuntTarget()
{
    AMyCharacter* MyChar = Cast<AMyCharacter>(GetOwner());
    if (!MyChar || !MyChar->IsLocallyControlled() || !MyChar->AutoMoveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AutoMove] MyChar or AutoMoveComp null, or not locally controlled"));
        return;
    }

    if (!AutoMoveTargetTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AutoMove] AutoMoveTargetTable이 설정되지 않았습니다"));
        return;
    }

    UQuestSubsystem* QuestSub = GetQuestSubsystem();
    if (!QuestSub)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AutoMove] QuestSubsystem null"));
        return;
    }

    for (const FActiveQuestInfo& ActiveQuest : QuestSub->ActiveQuests)
    {
        FAutoMoveTargetData* TargetData = AutoMoveTargetTable->FindRow<FAutoMoveTargetData>(ActiveQuest.QuestID, TEXT("AutoMoveTarget"));
        if (!TargetData)
        {
            UE_LOG(LogTemp, Warning, TEXT("[AutoMove] AutoMoveTargetTable에 [%s] 행 없음"), *ActiveQuest.QuestID.ToString());
            continue;
        }

        if (!ActiveQuest.bIsReadyToComplete)
        {
            // 사냥 단계: 목표 레벨과 현재 레벨을 비교해 이동 방법을 결정합니다.
            const FName TargetLevel  = TargetData->HuntTargetLevelName;
            const FName CurrentLevel = MyChar->CurrentZoneName;

            // 목표 레벨이 없거나 현재 레벨과 같으면 기존 방식으로 이동합니다.
            if (TargetLevel.IsNone() || TargetLevel == CurrentLevel)
            {
                UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 레벨 판별: 같은 레벨 → 현재=[%s] / 목표=[%s]"),
                    *CurrentLevel.ToString(), TargetLevel.IsNone() ? TEXT("None(같은레벨)") : *TargetLevel.ToString());

                if (!TargetData->HuntTargetLocation.IsZero())
                {
                    UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 사냥 목적지로 이동: %s"), *TargetData->HuntTargetLocation.ToString());
                    MyChar->AutoMoveComp->StartAutoMove(TargetData->HuntTargetLocation);
                    return;
                }
                UE_LOG(LogTemp, Warning, TEXT("[AutoMove] HuntTargetLocation이 (0,0,0)입니다. DataTable을 확인하세요."));
            }
            else
            {
                // 다른 레벨이면 BFS로 최단 경로를 탐색하고 첫 번째 포탈로 이동합니다.
                UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 레벨 판별: 다른 레벨 → 현재=[%s] / 목표=[%s] — BFS 탐색 시작"),
                    *CurrentLevel.ToString(), *TargetLevel.ToString());

                if (!LevelGraphTable)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[AutoMove] LevelGraphTable이 설정되지 않았습니다"));
                    return;
                }

                // ── BFS: CurrentLevel → TargetLevel 최단 경로 탐색 ──────────────────
                // visited: 방문한 레벨, parent: 경로 역추적용 부모 맵
                TMap<FName, FName> Parent;
                TQueue<FName>      Queue;
                TSet<FName>        Visited;

                Queue.Enqueue(CurrentLevel);
                Visited.Add(CurrentLevel);
                Parent.Add(CurrentLevel, FName("__ROOT__"));

                TArray<FLevelConnectionData*> AllEdges;
                LevelGraphTable->GetAllRows<FLevelConnectionData>(TEXT("BFS"), AllEdges);

                bool bFound = false;
                while (!Queue.IsEmpty())
                {
                    FName Current;
                    Queue.Dequeue(Current);

                    if (Current == TargetLevel)
                    {
                        bFound = true;
                        break;
                    }

                    for (FLevelConnectionData* Edge : AllEdges)
                    {
                        if (!Edge) continue;
                        if (Edge->FromLevel == Current && !Visited.Contains(Edge->ToLevel))
                        {
                            Visited.Add(Edge->ToLevel);
                            Parent.Add(Edge->ToLevel, Current);
                            Queue.Enqueue(Edge->ToLevel);
                        }
                    }
                }

                if (!bFound)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[AutoMove] BFS: [%s] → [%s] 경로를 찾지 못했습니다. LevelGraphTable을 확인하세요."),
                        *CurrentLevel.ToString(), *TargetLevel.ToString());
                    return;
                }

                // 역추적: TargetLevel부터 거슬러 올라가 전체 경로를 구성합니다.
                TArray<FName> FullPath;
                FName Step = TargetLevel;
                while (Step != FName("__ROOT__"))
                {
                    FullPath.Insert(Step, 0);
                    const FName* ParentPtr = Parent.Find(Step);
                    if (!ParentPtr) break;
                    Step = *ParentPtr;
                }

                // 전체 경로를 순서대로 로그로 출력합니다.
                FString PathStr;
                for (int32 i = 0; i < FullPath.Num(); ++i)
                {
                    PathStr += FullPath[i].ToString();
                    if (i < FullPath.Num() - 1) PathStr += TEXT(" → ");
                }
                UE_LOG(LogTemp, Warning, TEXT("[AutoMove] BFS 최적 경로: %s"), *PathStr);
                for (int32 i = 0; i < FullPath.Num(); ++i)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[AutoMove]   [%d] %s"), i + 1, *FullPath[i].ToString());
                }

                // CurrentLevel 바로 다음 레벨(첫 번째 이동 목표)을 구합니다.
                FName NextLevel = TargetLevel;
                while (Parent.FindRef(NextLevel) != CurrentLevel)
                {
                    NextLevel = Parent.FindRef(NextLevel);
                }

                UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 첫 번째 이동 목표 레벨: [%s] → 해당 포탈 탐색 중..."), *NextLevel.ToString());

                // 현재 레벨에서 NextLevel로 향하는 포탈을 검색합니다.
                TArray<AActor*> PortalActors;
                UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMapPortal::StaticClass(), PortalActors);

                for (AActor* Actor : PortalActors)
                {
                    AMapPortal* Portal = Cast<AMapPortal>(Actor);
                    if (!Portal) continue;

                    if (Portal->GetCurrentSubLevelName() != CurrentLevel) continue;

                    FName PortalTargetLevel = Portal->GetTargetSubLevelName();
                    if (PortalTargetLevel == NextLevel)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 포탈 발견 → 위치: %s"), *Portal->GetActorLocation().ToString());
                        MyChar->AutoMoveComp->StartAutoMove(Portal->GetActorLocation());
                        return;
                    }
                }

                UE_LOG(LogTemp, Warning, TEXT("[AutoMove] [%s]로 가는 포탈을 현재 레벨에서 찾지 못했습니다."), *NextLevel.ToString());
            }
        }
        else
        {
            // 완료 보고 단계: CompletionNPCTag로 레벨에서 NPC를 찾아 이동합니다.
            if (!TargetData->CompletionNPCTag.IsNone())
            {
                TArray<AActor*> TaggedActors;
                UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetData->CompletionNPCTag, TaggedActors);

                if (TaggedActors.Num() > 0)
                {
                    FVector NPCLocation = TaggedActors[0]->GetActorLocation();
                    UE_LOG(LogTemp, Warning, TEXT("[AutoMove] NPC 위치로 이동: %s"), *NPCLocation.ToString());
                    MyChar->AutoMoveComp->StartAutoMove(NPCLocation);
                    return;
                }
                UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 태그 [%s]를 가진 NPC를 찾지 못했습니다"), *TargetData->CompletionNPCTag.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[AutoMove] CompletionNPCTag가 비어있습니다. DataTable을 확인하세요."));
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[AutoMove] 이동 가능한 활성 퀘스트 없음"));
}
