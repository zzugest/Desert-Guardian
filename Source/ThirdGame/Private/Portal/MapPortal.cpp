// =========================================================================================
// MapPortal.cpp
//
// [파일 역할]
// 레벨에 배치하는 포탈 액터입니다. DataTable의 FPortalData를 읽어 두 가지 동작을 지원합니다.
//   - LevelTransition   : 확인창(PortalConfirmWidget)을 띄운 뒤 다른 레벨로 전환합니다.
//   - SameLevelTeleport : 지정 태그를 가진 액터 위치로 즉시 텔레포트합니다.
//
// 포탈 활성화 조건:
//   - PrerequisiteQuestID : 해당 퀘스트를 완료해야 포탈이 활성화됩니다.
//   - PrerequisiteBossRowName : 해당 보스가 처치돼야 포탈이 활성화됩니다.
// 조건이 충족되지 않으면 포탈을 숨기고 충돌을 비활성화합니다.
// =========================================================================================

#include "Portal/MapPortal.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MyCharacter.h"
#include "Portal/PortalData.h"
#include "Blueprint/UserWidget.h"
#include "Portal/PortalConfirmWidget.h"
#include "NPC/Quest/QuestComponent.h"
#include "Enemy/Enemy.h"
#include "MinimapSubsystem.h"

// 충돌 박스와 상호작용 프롬프트 위젯 컴포넌트를 생성합니다.
AMapPortal::AMapPortal()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;
    CollisionBox->SetCollisionProfileName(TEXT("Trigger"));

    // Screen 공간 위젯으로 설정해 카메라 방향과 무관하게 항상 화면을 향합니다.
    InteractPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractPromptWidget"));
    InteractPromptWidget->SetupAttachment(RootComponent);
    InteractPromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
    InteractPromptWidget->SetDrawSize(FVector2D(150.f, 50.f));
    InteractPromptWidget->SetVisibility(false);
}

// Overlap 이벤트를 바인딩하고, 퀘스트 완료·보스 처치 델리게이트를 연결한 뒤 초기 활성화 상태를 결정합니다.
void AMapPortal::BeginPlay()
{
    Super::BeginPlay();

    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AMapPortal::OnOverlapBegin);
    CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AMapPortal::OnOverlapEnd);

    // 퀘스트 완료 시 포탈 활성화를 다시 확인하기 위해 QuestComponent 델리게이트를 구독합니다.
    AMyCharacter* PlayerChar = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (PlayerChar && PlayerChar->QuestComponent)
    {
        PlayerChar->QuestComponent->OnQuestUIUpdated.AddDynamic(this, &AMapPortal::CheckAndApplyPortalState);
    }

    // 선행 보스가 설정된 경우 해당 보스의 OnMonsterDied 델리게이트를 구독합니다.
    if (PortalDataTable && !PortalRowName.IsNone())
    {
        FPortalData* PortalData = PortalDataTable->FindRow<FPortalData>(PortalRowName, TEXT("BossSetup"));
        if (PortalData && !PortalData->PrerequisiteBossRowName.IsNone())
        {
            TArray<AActor*> AllEnemies;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), AllEnemies);

            for (AActor* Actor : AllEnemies)
            {
                AEnemy* Enemy = Cast<AEnemy>(Actor);
                if (Enemy && Enemy->EnemyRowName == PortalData->PrerequisiteBossRowName)
                {
                    Enemy->OnMonsterDied.AddDynamic(this, &AMapPortal::OnBossKilled);
                    break;
                }
            }
        }
    }

    // 게임 시작 시 조건을 검사해 포탈 표시 여부를 결정합니다.
    CheckAndApplyPortalState();
}

// 플레이어가 포탈 범위에 들어오면 상호작용 프롬프트를 표시합니다.
void AMapPortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<AMyCharacter>())
    {
        InteractPromptWidget->SetVisibility(true);
    }
}

// 플레이어가 포탈 범위를 벗어나면 프롬프트를 숨깁니다.
void AMapPortal::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<AMyCharacter>())
    {
        InteractPromptWidget->SetVisibility(false);
    }
}

// 플레이어가 상호작용 키(F)를 누를 때 포탈 타입에 따라 동작을 분기합니다.
void AMapPortal::InteractWithPortal(AMyCharacter* PlayerCharacter)
{
    if (!PlayerCharacter) return;
    if (!PortalDataTable || PortalRowName.IsNone()) return;

    FPortalData* PortalData = PortalDataTable->FindRow<FPortalData>(PortalRowName, TEXT("Portal Context"));
    if (!PortalData) return;

    // ── 같은 레벨 내 텔레포트 ──────────────────────────────────────────
    if (PortalData->PortalType == EPortalType::SameLevelTeleport)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PORTAL] SameLevelTeleport 시작 | TargetActorTag: %s"), *PortalData->TargetActorTag.ToString());

        if (PortalData->TargetActorTag.IsNone())
        {
            UE_LOG(LogTemp, Error, TEXT("[PORTAL] 실패: TargetActorTag가 비어 있습니다. 데이터 테이블을 확인하세요."));
            return;
        }

        // 지정 태그를 가진 액터(TargetPoint 등)를 검색해 해당 위치로 즉시 이동합니다.
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), PortalData->TargetActorTag, FoundActors);

        UE_LOG(LogTemp, Warning, TEXT("[PORTAL] 태그 '%s' 검색 결과: %d개 액터 발견"), *PortalData->TargetActorTag.ToString(), FoundActors.Num());

        if (FoundActors.Num() == 0)
        {
            UE_LOG(LogTemp, Error, TEXT("[PORTAL] 실패: 태그 '%s'를 가진 액터를 찾지 못했습니다. PlayerStart에 태그가 정확히 설정됐는지 확인하세요."), *PortalData->TargetActorTag.ToString());
            return;
        }

        FVector  DestLocation = FoundActors[0]->GetActorLocation();
        FRotator DestRotation = FoundActors[0]->GetActorRotation();

        UE_LOG(LogTemp, Warning, TEXT("[PORTAL] 텔레포트 실행 | 목적지: %s | 회전: %s"), *DestLocation.ToString(), *DestRotation.ToString());

        PlayerCharacter->Server_TeleportTo(DestLocation, DestRotation);

        UE_LOG(LogTemp, Warning, TEXT("[PORTAL] 텔레포트 완료"));

        InteractPromptWidget->SetVisibility(false);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[PORTAL] PortalType이 SameLevelTeleport가 아닙니다. 현재 타입: %d | 데이터 테이블의 PortalType을 확인하세요."), (int32)PortalData->PortalType);

    // ── 레벨 전환: 확인창을 띄우고 플레이어 입력을 잠급니다 ─────────────
    if (!ConfirmWidgetClass) return;

    UPortalConfirmWidget* ConfirmUI = CreateWidget<UPortalConfirmWidget>(GetWorld(), ConfirmWidgetClass);
    if (!ConfirmUI) return;

    ConfirmUI->InitConfirmUI(*PortalData);
    ConfirmUI->AddToViewport();

    APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
    if (PC)
    {
        // UI 전용 입력 모드로 전환하고 마우스 커서를 표시합니다.
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(ConfirmUI->TakeWidget());
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
        // 확인창이 닫힐 때까지 캐릭터 이동 입력을 차단합니다.
        PlayerCharacter->DisableInput(PC);
    }

    InteractPromptWidget->SetVisibility(false);
}

// 선행 조건(퀘스트 완료 여부·보스 처치 여부)을 검사해 포탈의 표시·충돌을 동적으로 적용합니다.
void AMapPortal::CheckAndApplyPortalState()
{
    if (!PortalDataTable || PortalRowName.IsNone()) return;

    FPortalData* PortalData = PortalDataTable->FindRow<FPortalData>(PortalRowName, TEXT("CheckPortalState"));
    if (!PortalData) return;

    bool bShouldBeActive = true;

    // 선행 퀘스트가 설정된 경우 완료 여부를 확인합니다.
    if (!PortalData->PrerequisiteQuestID.IsNone())
    {
        AMyCharacter* PlayerChar = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
        if (PlayerChar && PlayerChar->QuestComponent)
        {
            bShouldBeActive = PlayerChar->QuestComponent->IsQuestCompleted(PortalData->PrerequisiteQuestID);
        }
        else
        {
            bShouldBeActive = false;
        }
    }

    // 선행 보스가 설정된 경우 처치 여부를 추가로 확인합니다.
    if (bShouldBeActive && !PortalData->PrerequisiteBossRowName.IsNone())
    {
        bShouldBeActive = bBossKilled;
    }

    // 조건 충족 시 포탈을 표시하고 충돌을 활성화, 미충족 시 숨기고 비활성화합니다.
    UMinimapSubsystem* MinimapSys = nullptr;
    if (UGameInstance* GI = GetGameInstance())
    {
        MinimapSys = GI->GetSubsystem<UMinimapSubsystem>();
    }

    if (bShouldBeActive)
    {
        SetActorHiddenInGame(false);
        SetActorEnableCollision(true);

        // 포탈이 활성화될 때 미니맵에 파란 점으로 등록합니다.
        if (MinimapSys) MinimapSys->RegisterMarker(this, EMinimapMarkerType::Portal);
    }
    else
    {
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        if (InteractPromptWidget) InteractPromptWidget->SetVisibility(false);

        // 포탈이 비활성화될 때 미니맵에서 제거합니다.
        if (MinimapSys) MinimapSys->UnregisterMarker(this);
    }
}

// 레벨 종료 시 미니맵 마커를 해제합니다.
void AMapPortal::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UMinimapSubsystem* MinimapSys = GI->GetSubsystem<UMinimapSubsystem>())
        {
            MinimapSys->UnregisterMarker(this);
        }
    }

    Super::EndPlay(EndPlayReason);
}

// 보스 처치 델리게이트 콜백: 보스 사망 플래그를 세우고 포탈 상태를 재검사합니다.
void AMapPortal::OnBossKilled(AEnemy* DeadEnemy)
{
    bBossKilled = true;
    CheckAndApplyPortalState();
}
