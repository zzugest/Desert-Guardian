// =========================================================================================
// BaseNPC.cpp
//
// [파일 역할]
// 모든 NPC(TalkNPC, QuestNPC, ShopNPC)의 공통 기반 클래스입니다.
// 스켈레탈 메시, 상호작용 박스(InteractionZone), 화면 공간 프롬프트 위젯을 생성하고,
// 플레이어가 범위에 들어오면 프롬프트를 표시·숨김 처리합니다.
// SetupDialogueState는 대화창을 열 때 이동 차단·마우스 활성화·기존 UI 닫기를 일괄 처리합니다.
// =========================================================================================

#include "NPC/BaseNPC.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "MyCharacter.h"
#include "UISubsystem.h"
#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"

// 메시, 상호작용 박스, 화면 공간 프롬프트 위젯을 생성하고 기본값을 설정합니다.
ABaseNPC::ABaseNPC()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    InteractionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionZone"));
    InteractionZone->SetupAttachment(RootComponent);
    InteractionZone->SetBoxExtent(FVector(100.f, 100.f, 100.f));

    InteractPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractPromptWidget"));
    InteractPromptWidget->SetupAttachment(RootComponent);

    // Screen 공간으로 설정해 카메라 회전과 무관하게 항상 화면을 향하도록 합니다.
    InteractPromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
    InteractPromptWidget->SetDrawSize(FVector2D(150.f, 50.f));

    // 처음에는 프롬프트를 숨겨두고 플레이어가 범위에 들어올 때만 표시합니다.
    InteractPromptWidget->SetVisibility(false);

    static ConstructorHelpers::FClassFinder<UUserWidget> InteractWidgetClass(
        TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/NPC/InteractPromptWidget.InteractPromptWidget_C'"));
    if (InteractWidgetClass.Succeeded())
    {
        InteractPromptWidget->SetWidgetClass(InteractWidgetClass.Class);
    }
}

// 상호작용 박스 Overlap 이벤트를 바인딩합니다.
void ABaseNPC::BeginPlay()
{
    Super::BeginPlay();

    if (InteractionZone)
    {
        InteractionZone->OnComponentBeginOverlap.AddDynamic(this, &ABaseNPC::OnInteractZoneBeginOverlap);
        InteractionZone->OnComponentEndOverlap.AddDynamic(this, &ABaseNPC::OnInteractZoneEndOverlap);
    }
}

// 서브클래스가 재정의하는 상호작용 진입점입니다. 기본 구현은 비어 있습니다.
void ABaseNPC::InteractWithPlayer(AMyCharacter* PlayerCharacter)
{
}

// 대화창을 열 때 공통적으로 필요한 상태를 일괄 설정합니다.
// (기존 UI 닫기 → 이동 차단 → UI 전용 입력 모드 → 마우스 표시)
void ABaseNPC::SetupDialogueState(AMyCharacter* PlayerCharacter, UUserWidget* DialogueWidget)
{
    if (!PlayerCharacter || !DialogueWidget) return;

    // 대화 중 다른 UI(인벤토리, 상점 등)가 열려 있으면 먼저 닫습니다.
    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (UUISubsystem* UISys = GI->GetSubsystem<UUISubsystem>())
        {
            UISys->CloseAllActiveUIs();
        }
    }

    APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
    if (PC)
    {
        // 이동 중이면 즉시 멈추고 이동 입력을 차단합니다.
        if (PlayerCharacter->GetCharacterMovement())
        {
            PlayerCharacter->GetCharacterMovement()->StopMovementImmediately();
        }
        PC->FlushPressedKeys();
        PC->SetIgnoreMoveInput(true);

        // UI 전용 입력 모드로 전환하고 마우스 커서를 표시합니다.
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(DialogueWidget->TakeWidget());
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    // 대화가 시작됐으므로 상호작용 프롬프트를 숨깁니다.
    if (InteractPromptWidget)
    {
        InteractPromptWidget->SetVisibility(false);
    }
}

// 플레이어가 상호작용 범위에 들어오면 프롬프트를 표시합니다.
void ABaseNPC::OnInteractZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<AMyCharacter>())
    {
        InteractPromptWidget->SetVisibility(true);
    }
}

// 플레이어가 상호작용 범위를 벗어나면 프롬프트를 숨깁니다.
void ABaseNPC::OnInteractZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<AMyCharacter>())
    {
        InteractPromptWidget->SetVisibility(false);
    }
}
