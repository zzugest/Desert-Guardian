// =========================================================================================
// MyCharacter.cpp
//
// [파일 역할]
// 플레이어가 조종하는 메인 캐릭터 클래스입니다.
//
// [주요 기능]
// 1. 컴포넌트 초기화
//    - CombatComponent   : 전투(공격, 피격, 버프, 마법)
//    - SkillComponent    : 스킬 습득 및 발동
//    - InventoryComponent: 아이템 보관
//    - QuickSlotComponent: 1~5번 단축 슬롯
//    - TargetingComponent: 화면 중앙 기준 적 자동 타겟팅
//    - MinimapComponent  : SceneCapture2D 기반 미니맵
//    - QuestComponent    : 퀘스트 진행 상태 관리
//
// 2. 입력 처리 (Enhanced Input)
//    - 이동 / 카메라 회전 / 공격 / 마법 / 상호작용
//    - 인벤토리·스킬창 토글 / 퀵슬롯(1~5) / 스킬(Q·E·R)
//    - 점프 / 대시·구르기 / 달리기
//
// 3. 상태 관리 (GameplayTag)
//    - State.Action.*    : 공격, 구르기, 마법 시전 등 행동 상태
//    - State.Movement.*  : 지상(Grounded) / 공중(InAir)
//    - State.Stance.*    : 전투 자세(Combat)
//    - State.CC.*        : 스턴 등 군중 제어
//    - State.Cutscene    : 컷신 재생 중 (타겟팅·입력 차단용)
// =========================================================================================


#include "MyCharacter.h"
#include "MinimapComponent.h"
#include "MinimapWidget.h"
#include "Character/HUDRootWidget.h"
#include "InventoryComponent.h"
#include "QuickSlotComponent.h"
#include "CombatComponent.h"
#include "PickableItem.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerHUDWidget.h"
#include "InventoryWidget.h"
#include "InventorySubsystem.h"
#include "QuickSlotSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/Shop/ShopNPC.h"
#include "UISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagsManager.h"
#include "Skill/SkillWindowWidget.h"
#include "NPC/Quest/QuestComponent.h"
#include "TargetingComponent.h"
#include "UndodgeableDamageType.h"
#include "Engine/DamageEvents.h"
#include "MapPortal.h"
#include "Blueprint/UserWidget.h"

// 캐릭터의 이동, 카메라, 전투/인벤토리 등 모든 컴포넌트를 생성하고 기본값을 설정합니다.
AMyCharacter::AMyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 카메라가 마우스를 따라가도 캐릭터 본체는 컨트롤러 회전을 그대로 따르지 않도록 분리
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    QuestComponent = CreateDefaultSubobject<UQuestComponent>(TEXT("QuestComponent"));

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel5, ECR_Block);
    GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 600.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 150.0f);
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10.0f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    QuickSlotComp = CreateDefaultSubobject<UQuickSlotComponent>(TEXT("QuickSlotComp"));
    CombatComp    = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
    MoneyComp     = CreateDefaultSubobject<UMoneyComponent>(TEXT("MoneyComponent"));
    SkillComp     = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComp"));
    TargetingComp = CreateDefaultSubobject<UTargetingComponent>(TEXT("TargetingComponent"));
    MinimapComp   = CreateDefaultSubobject<UMinimapComponent>(TEXT("MinimapComp"));
}

// 게임 시작 시 입력 매핑, HUDRoot(통합 HUD) 생성, 인벤토리 위젯 및 서브시스템 이벤트 연결을 수행합니다.
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 1. Enhanced Input 매핑 컨텍스트 등록
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (PC)
    {
        UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
        if (Subsystem && DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    // 2. 루트 HUD 생성 — PlayerHUD, MinimapWidget, QuestLogWidget을 하나로 묶음
    //    HUDRoot 하나만 숨기면 모든 자식 HUD가 함께 사라지므로 컷신 처리에 유리합니다.
    if (HUDRootClass)
    {
        HUDRoot = CreateWidget<UHUDRootWidget>(PC, HUDRootClass);
        if (HUDRoot)
        {
            HUDRoot->AddToViewport();

            // 자식 포인터 획득 — 기존 CombatComponent 등의 코드가 그대로 동작
            PlayerHUD    = HUDRoot->PlayerHUD;
            MinimapWidget = HUDRoot->MinimapWidget;

            if (PlayerHUD && CombatComp)
            {
                PlayerHUD->UpdateState(CombatComp->CurrentHP, CombatComp->MaxHP, CombatComp->CurrentMP, CombatComp->MaxMP, CombatComp->CurrentSP, CombatComp->MaxSP);
            }

            if (MinimapWidget && MinimapComp)
            {
                MinimapWidget->SetRenderTarget(MinimapComp->GetRenderTarget());
                MinimapWidget->MapRange = MinimapComp->MapRange;
            }
        }
    }

    // 3. 인벤토리 위젯 미리 생성 (나중에 I 키를 누를 때 바로 표시)
    if (InventoryWidgetClass)
    {
        InventoryWidget = CreateWidget<UInventoryWidget>(GetWorld(), InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->OnWindowClosed.AddDynamic(this, &AMyCharacter::ForceCloseInventory);
        }
    }

    // 4. 인벤토리 서브시스템 갱신 이벤트 연결
    UGameInstance* GI = GetGameInstance();
    if (GI)
    {
        UInventorySubsystem* InvSubsystem = GI->GetSubsystem<UInventorySubsystem>();
        if (InvSubsystem && !InvSubsystem->OnInventoryUpdated.IsAlreadyBound(this, &AMyCharacter::OnInventoryUpdated))
        {
            InvSubsystem->OnInventoryUpdated.AddDynamic(this, &AMyCharacter::OnInventoryUpdated);
        }
    }

    // QuestLogWidget은 WBP_HUDRoot의 자식으로 통합되었으므로 별도 AddToViewport가 필요 없습니다.
    // MinimapWidget도 HUDRoot 생성 시 함께 초기화됩니다. (위 2번 블록 참고)
}

// 캐릭터가 파괴될 때 활성 타이머를 해제하고, 서브시스템 델리게이트 바인딩을 정리합니다.
void AMyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // 전투 대기 타이머가 활성 중일 때 레벨 전환/파괴되면 타이머를 명시적으로 해제합니다.
    GetWorldTimerManager().ClearTimer(CombatTimerHandle);

    // InventorySubsystem은 레벨이 바뀌어도 살아있으므로, MyCharacter가 파괴될 때
    // 직접 바인딩을 해제하지 않으면 파괴된 객체의 함수를 호출해 크래시가 발생할 수 있습니다.
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UInventorySubsystem* InvSubsystem = GI->GetSubsystem<UInventorySubsystem>())
        {
            InvSubsystem->OnInventoryUpdated.RemoveDynamic(this, &AMyCharacter::OnInventoryUpdated);
        }
    }
}

// 매 프레임 공중/지상 GameplayTag를 갱신하고, 달리기 중 스태미나 소모 및 자동 회복을 처리합니다.
void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 낙하 여부에 따라 InAir / Grounded 태그를 갱신합니다.
    if (GetCharacterMovement()->IsFalling())
    {
        if (!HasStateTag("State.Movement.InAir"))
        {
            AddStateTag("State.Movement.InAir");
            RemoveStateTag("State.Movement.Grounded");
        }
    }
    else
    {
        if (!HasStateTag("State.Movement.Grounded"))
        {
            AddStateTag("State.Movement.Grounded");
            RemoveStateTag("State.Movement.InAir");
        }
    }

    if (!CombatComp) return;

    // 달리기 중 스태미나 소모 처리
    if (bIsSprinting)
    {
        // 공중이거나 공격 중일 때는 달리기 강제 종료
        if (HasStateTag("State.Movement.InAir") || HasStateTag("State.Action.Attacking"))
        {
            StopSprint();
        }
        else
        {
            float SprintCost = 15.0f;
            CombatComp->CurrentSP -= (SprintCost * DeltaTime);

            if (CombatComp->CurrentSP <= 0.0f)
            {
                CombatComp->CurrentSP = 0.0f;
                StopSprint();
            }
        }
    }
    // 달리기 미사용 시 스태미나 자동 회복
    else
    {
        if (!HasStateTag("State.Action.Rolling") && CombatComp->CurrentSP < CombatComp->MaxSP)
        {
            float RegenRate = 10.0f;
            CombatComp->CurrentSP += (RegenRate * DeltaTime);
            if (CombatComp->CurrentSP > CombatComp->MaxSP)
            {
                CombatComp->CurrentSP = CombatComp->MaxSP;
            }
        }
    }

    if (PlayerHUD)
    {
        PlayerHUD->UpdateState(CombatComp->CurrentHP, CombatComp->MaxHP, CombatComp->CurrentMP, CombatComp->MaxMP, CombatComp->CurrentSP, CombatComp->MaxSP);
    }
}

// Enhanced Input 액션과 각 기능별 함수를 바인딩합니다.
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInputComponent) return;

    if (MoveAction)   EnhancedInputComponent->BindAction(MoveAction,   ETriggerEvent::Triggered, this, &AMyCharacter::Move);
    if (LookAction)   EnhancedInputComponent->BindAction(LookAction,   ETriggerEvent::Triggered, this, &AMyCharacter::Look);
    if (AttackAction) EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started,   this, &AMyCharacter::Attack);
    if (MagicAttackAction) EnhancedInputComponent->BindAction(MagicAttackAction, ETriggerEvent::Started, this, &AMyCharacter::MagicAttack);
    if (InteractAction)    EnhancedInputComponent->BindAction(InteractAction,    ETriggerEvent::Started, this, &AMyCharacter::Interact);
    if (InventoryAction)   EnhancedInputComponent->BindAction(InventoryAction,   ETriggerEvent::Started, this, &AMyCharacter::ToggleInventory);

    if (JumpAction)
    {
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started,   this, &AMyCharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
    }

    if (QuickSlot1Action) EnhancedInputComponent->BindAction(QuickSlot1Action, ETriggerEvent::Started, this, &AMyCharacter::OnQuickSlot1);
    if (QuickSlot2Action) EnhancedInputComponent->BindAction(QuickSlot2Action, ETriggerEvent::Started, this, &AMyCharacter::OnQuickSlot2);
    if (QuickSlot3Action) EnhancedInputComponent->BindAction(QuickSlot3Action, ETriggerEvent::Started, this, &AMyCharacter::OnQuickSlot3);
    if (QuickSlot4Action) EnhancedInputComponent->BindAction(QuickSlot4Action, ETriggerEvent::Started, this, &AMyCharacter::OnQuickSlot4);
    if (QuickSlot5Action) EnhancedInputComponent->BindAction(QuickSlot5Action, ETriggerEvent::Started, this, &AMyCharacter::OnQuickSlot5);

    if (SkillQAction) EnhancedInputComponent->BindAction(SkillQAction, ETriggerEvent::Started, this, &AMyCharacter::UseSkillQ);
    if (SkillEAction) EnhancedInputComponent->BindAction(SkillEAction, ETriggerEvent::Started, this, &AMyCharacter::UseSkillE);
    if (SkillRAction) EnhancedInputComponent->BindAction(SkillRAction, ETriggerEvent::Started, this, &AMyCharacter::UseSkillR);

    if (OpenSkillWindowAction) EnhancedInputComponent->BindAction(OpenSkillWindowAction, ETriggerEvent::Started, this, &AMyCharacter::ToggleSkillWindow);

    if (SprintAction)
    {
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started,   this, &AMyCharacter::StartSprint);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AMyCharacter::StopSprint);
    }
}

// 좌클릭 공격 입력을 CombatComponent로 전달합니다.
void AMyCharacter::Attack(const FInputActionValue& Value)
{
    if (!CombatComp) return;
    CombatComp->Attack();
}

// 애니메이션 몽타주 중간에 발생하는 콤보 입력을 CombatComponent로 전달합니다.
void AMyCharacter::ProcessComboCommand()
{
    if (!CombatComp) return;
    CombatComp->ProcessComboCommand();
}

// 인벤토리 내용이 변경되었을 때 UI가 열려 있다면 최신 데이터로 갱신합니다.
void AMyCharacter::OnInventoryUpdated()
{
    if (!InventoryWidget || !InventoryWidget->IsInViewport()) return;

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UInventorySubsystem* InvSubsystem = GI->GetSubsystem<UInventorySubsystem>();
    if (!InvSubsystem) return;

    InventoryWidget->UpdateUI(InvSubsystem->Content, InvSubsystem->Capacity);
}

// 캐릭터와 겹치기 시작한 아이템(Pickable)을 저장해 상호작용 대기 상태로 만듭니다.
void AMyCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    APickableItem* Pickable = Cast<APickableItem>(OtherActor);
    if (Pickable)
    {
        OverlappingItem = Pickable;
    }
}

// 겹치기가 끝난 아이템이 현재 저장된 대상이면 참조를 비웁니다.
void AMyCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
    Super::NotifyActorEndOverlap(OtherActor);

    if (OtherActor == OverlappingItem)
    {
        OverlappingItem = nullptr;
    }
}

// 컨트롤러 카메라 방향을 기준으로 WASD 입력을 월드 좌표 이동으로 변환합니다.
// 스턴·지상 공격 중·착지 모션 중에는 이동이 차단됩니다.
void AMyCharacter::Move(const FInputActionValue& Value)
{
    if (HasStateTag("State.CC.Stun")) return;

    bool bIsGroundAttacking = HasStateTag("State.Action.Attacking") && !HasStateTag("State.Movement.InAir");
    bool bIsJumpLanding     = HasStateTag("State.Action.JumpLanding");

    if (bIsGroundAttacking || bIsJumpLanding) return;

    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        const FRotator Rotation    = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection,   MovementVector.X);
    }
}

// 마우스(또는 게임패드 스틱) 입력을 컨트롤러 Pitch/Yaw에 반영해 카메라 시점을 조작합니다.
void AMyCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

// F 키 입력 시 우선순위에 따라 아이템 줍기 → NPC 대화 → 맵 포탈 순서로 상호작용을 시도합니다.
void AMyCharacter::Interact(const FInputActionValue& Value)
{
    // 1순위: 겹쳐 있는 아이템 줍기
    if (OverlappingItem)
    {
        OverlappingItem->Interact(this);
        return;
    }

    // 2순위: 겹쳐 있는 NPC와 대화
    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors, ABaseNPC::StaticClass());

    for (AActor* Actor : OverlappingActors)
    {
        ABaseNPC* TargetNPC = Cast<ABaseNPC>(Actor);
        if (TargetNPC)
        {
            TargetNPC->InteractWithPlayer(this);
            return;
        }
    }

    // 3순위: 겹쳐 있는 맵 포탈 진입
    TArray<AActor*> OverlappingPortals;
    GetOverlappingActors(OverlappingPortals, AMapPortal::StaticClass());

    for (AActor* Actor : OverlappingPortals)
    {
        AMapPortal* TargetPortal = Cast<AMapPortal>(Actor);
        if (TargetPortal)
        {
            TargetPortal->InteractWithPortal(this);
            return;
        }
    }
}

// 인벤토리 창을 열거나 닫고, UISubsystem에 현재 열림/닫힘 상태를 보고합니다.
void AMyCharacter::ToggleInventory(const FInputActionValue& Value)
{
    if (!IsValid(InventoryWidget)) return;

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UUISubsystem* UISys = GI->GetSubsystem<UUISubsystem>();

    if (InventoryWidget->IsVisible())
    {
        ForceCloseInventory();
    }
    else
    {
        if (!InventoryWidget->IsInViewport())
        {
            InventoryWidget->AddToViewport();
        }

        InventoryWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        UInventorySubsystem* InvSubsystem = GI->GetSubsystem<UInventorySubsystem>();
        if (InvSubsystem)
        {
            InventoryWidget->UpdateUI(InvSubsystem->Content, InvSubsystem->Capacity);
        }

        if (UISys) UISys->ReportUIOpened(InventoryWidget);
    }
}

// 인벤토리 창을 즉시 숨기고 UISubsystem에 닫힘을 보고해 입력 모드를 복구합니다.
void AMyCharacter::ForceCloseInventory()
{
    if (!IsValid(InventoryWidget) || !InventoryWidget->IsVisible()) return;

    InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);

    UUISubsystem* UISys = GetGameInstance()->GetSubsystem<UUISubsystem>();
    if (UISys)
    {
        UISys->ReportUIClosed(InventoryWidget);
    }
}

// 숫자 키(1~5) 입력을 받아 해당 인덱스의 퀵슬롯 처리 함수로 전달합니다.
void AMyCharacter::OnQuickSlot1(const FInputActionValue& Value) { ProcessQuickSlot(0); }
void AMyCharacter::OnQuickSlot2(const FInputActionValue& Value) { ProcessQuickSlot(1); }
void AMyCharacter::OnQuickSlot3(const FInputActionValue& Value) { ProcessQuickSlot(2); }
void AMyCharacter::OnQuickSlot4(const FInputActionValue& Value) { ProcessQuickSlot(3); }
void AMyCharacter::OnQuickSlot5(const FInputActionValue& Value) { ProcessQuickSlot(4); }

// QuickSlotSubsystem에 지정 인덱스 슬롯의 아이템 사용을 요청합니다.
void AMyCharacter::ProcessQuickSlot(int32 SlotIndex)
{
    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UQuickSlotSubsystem* QuickSubsystem = GI->GetSubsystem<UQuickSlotSubsystem>();
    if (QuickSubsystem)
    {
        QuickSubsystem->UseQuickSlot(SlotIndex);
    }
}

// Q / E / R 키 입력을 받아 SkillComponent의 해당 슬롯 스킬 시전을 시도합니다.
void AMyCharacter::UseSkillQ(const FInputActionValue& Value) { if (SkillComp) SkillComp->TryCastSkill(0); }
void AMyCharacter::UseSkillE(const FInputActionValue& Value) { if (SkillComp) SkillComp->TryCastSkill(1); }
void AMyCharacter::UseSkillR(const FInputActionValue& Value) { if (SkillComp) SkillComp->TryCastSkill(2); }

// GameplayTag 컨테이너에 상태 태그를 추가 / 제거 / 확인합니다.
void AMyCharacter::AddStateTag(FName TagName)  { ActionTags.AddTag(FGameplayTag::RequestGameplayTag(TagName)); }
void AMyCharacter::RemoveStateTag(FName TagName) { ActionTags.RemoveTag(FGameplayTag::RequestGameplayTag(TagName)); }
bool AMyCharacter::HasStateTag(FName TagName)  { return ActionTags.HasTag(FGameplayTag::RequestGameplayTag(TagName)); }

// 공격·스턴·구르기 중에는 점프를 차단하고, 그 외에는 기본 점프를 실행합니다.
void AMyCharacter::Jump()
{
    if (HasStateTag("State.Action.Attacking") || HasStateTag("State.CC.Stun") || HasStateTag("State.Action.Rolling"))
    {
        return;
    }

    Super::Jump();
}

// K 키로 스킬 창을 열거나 닫습니다. 최초 열 때 위젯을 생성하고 닫기 델리게이트를 연결합니다.
void AMyCharacter::ToggleSkillWindow(const FInputActionValue& Value)
{
    // 아직 위젯이 없으면 최초 생성 후 닫기 이벤트 연결
    if (!IsValid(SkillWindowWidget))
    {
        APlayerController* PC = Cast<APlayerController>(GetController());
        if (PC && SkillWindowClass)
        {
            SkillWindowWidget = CreateWidget<USkillWindowWidget>(PC, SkillWindowClass);

            if (SkillWindowWidget)
            {
                SkillWindowWidget->OnWindowClosed.AddDynamic(this, &AMyCharacter::ForceCloseSkillWindow);
            }
        }
    }

    if (!IsValid(SkillWindowWidget)) return;

    UUISubsystem* UISys = GetGameInstance()->GetSubsystem<UUISubsystem>();

    if (SkillWindowWidget->IsVisible())
    {
        ForceCloseSkillWindow();
    }
    else
    {
        if (!SkillWindowWidget->IsInViewport())
        {
            SkillWindowWidget->AddToViewport();
        }

        // 창을 열 때마다 최신 스킬 데이터를 반영해 목록을 새로 그림
        SkillWindowWidget->RefreshUI();
        SkillWindowWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        if (UISys) UISys->ReportUIOpened(SkillWindowWidget);
    }
}

// 스킬 창을 즉시 숨기고 UISubsystem에 닫힘을 보고해 입력 모드를 복구합니다.
void AMyCharacter::ForceCloseSkillWindow()
{
    if (!IsValid(SkillWindowWidget)) return;
    if (!SkillWindowWidget->IsVisible()) return;

    SkillWindowWidget->SetVisibility(ESlateVisibility::Collapsed);

    UUISubsystem* UISys = GetGameInstance()->GetSubsystem<UUISubsystem>();
    if (UISys)
    {
        UISys->ReportUIClosed(SkillWindowWidget);
    }
}

// 이동 모드가 변경될 때 낙하 상태면 InAir 태그를 부여하고, 착지하면 제거합니다.
void AMyCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

    if (GetCharacterMovement()->MovementMode == MOVE_Falling)
    {
        AddStateTag(FName("State.Movement.InAir"));
    }
    else if (GetCharacterMovement()->MovementMode == MOVE_Walking || GetCharacterMovement()->MovementMode == MOVE_NavWalking)
    {
        RemoveStateTag(FName("State.Movement.InAir"));
    }
}

// 착지 시 InAir 태그를 제거하고 CombatComponent에 착지 애니메이션 재생을 요청합니다.
void AMyCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    RemoveStateTag(FName("State.Movement.InAir"));

    if (CombatComp)
    {
        CombatComp->PlayJumpLandingAnim();
    }
}

// 피격 시 구르기 무적 여부를 판단하고, 실제 피해를 CombatComponent에 전달해 HP를 감소시킵니다.
float AMyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 피해 타입이 '회피 불가(UndodgeableDamageType)'인지 확인
    TSubclassOf<UDamageType> DamageTypeClass = DamageEvent.DamageTypeClass;
    bool bIsUndodgeable = (DamageTypeClass == UUndodgeableDamageType::StaticClass());

    // 구르기 중이고 회피 불가 피해가 아니라면 무적 처리
    if (HasStateTag("State.Action.Rolling") && !bIsUndodgeable)
    {
        return 0.0f;
    }

    // 실제 피해 적용 및 HUD 갱신
    if (ActualDamage > 0.0f && CombatComp != nullptr)
    {
        EnterCombatStance();
        CombatComp->ReceiveDamage(ActualDamage);

        if (PlayerHUD != nullptr)
        {
            PlayerHUD->UpdateState(
                CombatComp->CurrentHP, CombatComp->MaxHP,
                CombatComp->CurrentMP, CombatComp->MaxMP,
                CombatComp->CurrentSP, CombatComp->MaxSP
            );
        }
    }

    return ActualDamage;
}

// Shift 키를 누르면 전투 자세면 구르기(8방향)를, 일반 상태면 달리기를 시작합니다.
void AMyCharacter::StartSprint()
{
    if (HasStateTag("State.Movement.InAir") || HasStateTag("State.Action.Attacking") || HasStateTag("State.Action.Rolling") || HasStateTag("State.Action.MagicCasting")) return;

    if (HasStateTag("State.Stance.Combat"))
    {
        // 전투 자세: 스태미나 소모 후 입력 방향에 따른 8방향 구르기 몽타주 재생
        float RollCost = 20.0f;

        if (CombatComp && CombatComp->CurrentSP >= RollCost)
        {
            CombatComp->CurrentSP -= RollCost;

            if (PlayerHUD != nullptr)
            {
                PlayerHUD->UpdateState(
                    CombatComp->CurrentHP, CombatComp->MaxHP,
                    CombatComp->CurrentMP, CombatComp->MaxMP,
                    CombatComp->CurrentSP, CombatComp->MaxSP
                );
            }

            UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
            if (AnimInstance && CombatRollMontage)
            {
                AddStateTag("State.Action.Rolling");

                FVector Accel = GetCharacterMovement()->GetCurrentAcceleration();
                FName SectionName = FName("Forward");

                // 입력 방향(가속도)과 캐릭터 전방·우측 벡터의 내적으로 8방향 섹션 결정
                if (!Accel.IsNearlyZero())
                {
                    Accel.Normalize();
                    float ForwardDot = FVector::DotProduct(GetActorForwardVector(), Accel);
                    float RightDot   = FVector::DotProduct(GetActorRightVector(),   Accel);

                    if (ForwardDot >= 0.5f)
                    {
                        SectionName = (RightDot > 0.5f) ? FName("ForwardRight") : (RightDot < -0.5f) ? FName("ForwardLeft") : FName("Forward");
                    }
                    else if (ForwardDot <= -0.5f)
                    {
                        SectionName = (RightDot > 0.5f) ? FName("BackwardRight") : (RightDot < -0.5f) ? FName("BackwardLeft") : FName("Backward");
                    }
                    else
                    {
                        SectionName = (RightDot > 0.0f) ? FName("Right") : FName("Left");
                    }
                }
                else
                {
                    SectionName = FName("Backward");
                }

                AnimInstance->Montage_Play(CombatRollMontage);
                AnimInstance->Montage_JumpToSection(SectionName, CombatRollMontage);

                FOnMontageEnded EndDelegate;
                EndDelegate.BindUObject(this, &AMyCharacter::OnRollMontageEnded);
                AnimInstance->Montage_SetEndDelegate(EndDelegate, CombatRollMontage);
            }
        }
        return;
    }

    // 일반 상태: 스태미나가 남아 있으면 이동 속도를 높여 달리기 시작 (소모는 Tick에서 처리)
    if (CombatComp && CombatComp->CurrentSP > 0.0f)
    {
        bIsSprinting = true;
        GetCharacterMovement()->MaxWalkSpeed = 800.0f;
    }
}

// Shift 키를 떼면 이동 속도를 기본값으로 복구하고 달리기 플래그를 해제합니다.
void AMyCharacter::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = 500.0f;
}

// 전투 자세로 전환해 캐릭터가 카메라 방향을 바라보게 하고, 일정 시간 후 자동 해제 타이머를 설정합니다.
void AMyCharacter::EnterCombatStance()
{
    AddStateTag("State.Stance.Combat");

    // 공격·피격이 없으면 CombatTimeout 후 자동으로 전투 자세 해제
    GetWorldTimerManager().SetTimer(CombatTimerHandle, this, &AMyCharacter::ExitCombatStance, CombatTimeout, false);

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = false;
        bUseControllerRotationYaw = true;
    }
}

// 전투 타이머 만료 후 전투 자세를 해제하고 이동 방향 자동 회전을 복구합니다.
void AMyCharacter::ExitCombatStance()
{
    RemoveStateTag("State.Stance.Combat");

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = true;
        bUseControllerRotationYaw = false;
    }
}

// 구르기 몽타주가 끝나면 구르기 상태 태그를 제거해 정상 상태로 복귀시킵니다.
void AMyCharacter::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    RemoveStateTag("State.Action.Rolling");
}

// 우클릭 마법 공격 입력을 CombatComponent로 전달합니다.
void AMyCharacter::MagicAttack()
{
    if (CombatComp)
    {
        CombatComp->RightClickMagicAttack();
    }
}
