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
#include "Item/PickableItem.h"
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
#include "Skill/SkillSubsystem.h"
#include "Skill/SkillData.h"
#include "NPC/Quest/QuestComponent.h"
#include "TargetingComponent.h"
#include "UndodgeableDamageType.h"
#include "Engine/DamageEvents.h"
#include "MapPortal.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"

// 복제할 변수를 등록합니다.
void AMyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyCharacter, bIsInCombatStance);
}

// 캐릭터의 이동, 카메라, 전투/인벤토리 등 모든 컴포넌트를 생성하고 기본값을 설정합니다.
AMyCharacter::AMyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 8000 유닛(80m) 밖의 다른 플레이어는 복제하지 않아 렌더링과 네트워크 대역폭을 절약합니다.
    NetCullDistanceSquared = 8000.0f * 8000.0f;

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
    InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComp"));
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

    // SP 소모/회복은 서버에서만 처리합니다. 변경된 값은 OnRep_Stats를 통해 클라이언트 HUD에 자동 반영됩니다.
    if (HasAuthority())
    {
        if (bIsSprinting)
        {
            // 공중이거나 공격 중일 때는 달리기 강제 종료
            if (HasStateTag("State.Movement.InAir") || HasStateTag("State.Action.Attacking"))
            {
                MulticastForceStopSprint();
            }
            else
            {
                float SprintCost = 15.0f;
                CombatComp->CurrentSP -= (SprintCost * DeltaTime);

                if (CombatComp->CurrentSP <= 0.0f)
                {
                    CombatComp->CurrentSP = 0.0f;
                    // SP 소진 시 모든 클라이언트에 달리기 중단을 알립니다.
                    MulticastForceStopSprint();
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
    }

    // HUD 갱신은 OnRep_Stats가 담당합니다. (Tick에서 매 프레임 호출 불필요)
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

// 좌클릭 공격 입력 시 타겟 방향을 계산해 서버에 공격을 요청합니다.
void AMyCharacter::Attack(const FInputActionValue& Value)
{
    if (!CombatComp) return;

    // 타겟이 있으면 그 방향, 없으면 현재 방향 그대로 전달합니다.
    FRotator SnapRotation = GetActorRotation();
    if (TargetingComp && TargetingComp->CurrentTarget)
    {
        FVector ToTarget = TargetingComp->CurrentTarget->GetActorLocation() - GetActorLocation();
        ToTarget.Z = 0.f;
        if (!ToTarget.IsNearlyZero())
        {
            SnapRotation = ToTarget.Rotation();
        }
    }

    ServerRequestAttack(SnapRotation);
}

// 서버에서 실행: 캐릭터를 지정 위치·회전으로 텔레포트합니다.
void AMyCharacter::Server_TeleportTo_Implementation(FVector DestLocation, FRotator DestRotation)
{
    SetActorLocationAndRotation(DestLocation, DestRotation,
        false, nullptr, ETeleportType::TeleportPhysics);
}

// 서버에서 실행: 목적지 서브레벨을 로드한 뒤 텔레포트하고, 이전 서브레벨을 언로드합니다.
// TargetSubLevelName이 None이면 Persistent Level(마을)이므로 바로 텔레포트합니다.
void AMyCharacter::Server_RequestPortalTravel_Implementation(FName TargetSubLevelName, FName UnloadSubLevelName, FVector Dest, FRotator Rot)
{
    PendingTravelLocation      = Dest;
    PendingTravelRotation      = Rot;
    PendingUnloadSubLevelName  = UnloadSubLevelName;
    PendingTargetSubLevelName  = TargetSubLevelName;

    if (TargetSubLevelName.IsNone())
    {
        // 목적지가 Persistent Level(마을)이면 로드 불필요 — 바로 텔레포트합니다.
        OnPortalLevelLoaded();
        return;
    }

    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget    = this;
    LatentInfo.ExecutionFunction = FName("OnPortalLevelLoaded");
    LatentInfo.UUID              = 7701;
    LatentInfo.Linkage           = 0;
    UGameplayStatics::LoadStreamLevel(this, TargetSubLevelName, true, false, LatentInfo);
}

// 목적지 서브레벨 로드 완료 콜백: 현재 존을 업데이트하고 텔레포트 후 이전 서브레벨을 언로드합니다.
void AMyCharacter::OnPortalLevelLoaded()
{
    // 현재 존 업데이트: 다른 플레이어의 레퍼런스 카운팅에 사용됩니다.
    CurrentZoneName = PendingTargetSubLevelName;

    SetActorLocationAndRotation(PendingTravelLocation, PendingTravelRotation,
        false, nullptr, ETeleportType::TeleportPhysics);

    if (!PendingUnloadSubLevelName.IsNone())
    {
        // 언로드 대상 서브레벨에 남아있는 플레이어 수를 카운트합니다.
        int32 PlayerCountInOldZone = 0;
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (!PC) continue;

            AMyCharacter* OtherChar = Cast<AMyCharacter>(PC->GetPawn());
            if (!OtherChar) continue;

            if (OtherChar->CurrentZoneName == PendingUnloadSubLevelName)
            {
                PlayerCountInOldZone++;
            }
        }

        // 해당 서브레벨에 남은 플레이어가 없을 때만 언로드합니다.
        if (PlayerCountInOldZone == 0)
        {
            FLatentActionInfo LatentInfo;
            LatentInfo.CallbackTarget    = this;
            LatentInfo.ExecutionFunction = FName("OnPortalUnloadComplete");
            LatentInfo.UUID              = 7702;
            LatentInfo.Linkage           = 0;
            UGameplayStatics::UnloadStreamLevel(this, PendingUnloadSubLevelName, LatentInfo, false);
        }
    }

    // 이동한 클라이언트 로컬 화면에서도 존 지오메트리를 교체합니다.
    // 다른 클라이언트에는 영향을 주지 않습니다.
    Client_UpdateZoneStreaming(PendingTargetSubLevelName, PendingUnloadSubLevelName);

    PendingTargetSubLevelName = NAME_None;
    PendingTravelLocation     = FVector::ZeroVector;
    PendingTravelRotation     = FRotator::ZeroRotator;
    PendingUnloadSubLevelName = NAME_None;
}

// 이전 서브레벨 언로드 완료 콜백: 현재는 별도 처리 없음.
void AMyCharacter::OnPortalUnloadComplete()
{
}

// 서버 → 해당 클라이언트 전용: 이 클라이언트 로컬에서만 존 지오메트리를 로드/언로드합니다.
// 다른 클라이언트에는 호출되지 않아 각자의 화면에 자신의 존만 렌더링됩니다.
void AMyCharacter::Client_UpdateZoneStreaming_Implementation(FName ToLoad, FName ToUnload)
{
    // 새 존 지오메트리를 이 클라이언트 로컬에서만 로드합니다.
    if (!ToLoad.IsNone())
    {
        FLatentActionInfo LatentInfo;
        LatentInfo.CallbackTarget    = this;
        LatentInfo.ExecutionFunction = FName("OnClientZoneLoaded");
        LatentInfo.UUID              = 7703;
        LatentInfo.Linkage           = 0;
        UGameplayStatics::LoadStreamLevel(this, ToLoad, true, false, LatentInfo);
    }

    // 이전 존 지오메트리를 이 클라이언트 로컬에서만 언로드합니다.
    if (!ToUnload.IsNone())
    {
        FLatentActionInfo LatentInfo;
        LatentInfo.CallbackTarget    = this;
        LatentInfo.ExecutionFunction = FName("OnClientZoneUnloaded");
        LatentInfo.UUID              = 7704;
        LatentInfo.Linkage           = 0;
        UGameplayStatics::UnloadStreamLevel(this, ToUnload, LatentInfo, false);
    }
}

// 클라이언트 로컬 존 로드 완료 콜백 — 현재는 별도 처리 없음.
void AMyCharacter::OnClientZoneLoaded()
{
}

// 클라이언트 로컬 존 언로드 완료 콜백 — 현재는 별도 처리 없음.
void AMyCharacter::OnClientZoneUnloaded()
{
}

// 서버에서 실행: 기존 Attack() 로직을 서버에서 처리한 뒤 클라이언트에 애님을 동기화합니다.
void AMyCharacter::ServerRequestAttack_Implementation(FRotator SnapRotation)
{
    if (!CombatComp) return;

    // 클라이언트가 전달한 스냅 방향을 저장합니다. (콤보 이어치기 Multicast에 재사용)
    LastSnapRotation = SnapRotation;

    // 서버에서도 몽타주 재생 전에 캐릭터를 스냅 방향으로 먼저 회전시킵니다.
    // 서버와 클라이언트가 동일한 초기 방향에서 루트 모션을 시작해야 CMC 위치 보정이 발생하지 않습니다.
    SetActorRotation(SnapRotation);

    // 공격 시작 전 상태를 기록해 실제로 새 공격이 시작됐는지 판단합니다.
    bool bWasAttacking = HasStateTag("State.Action.Attacking");

    // 기존 CombatComponent 공격 로직 전체를 서버에서 실행합니다.
    // (상태 체크, 콤보 카운터, 상태태그, 루트모션 설정, 몽타주 재생 포함)
    CombatComp->Attack();

    // 새 공격이 시작된 경우에만 Multicast로 클라이언트에 애님을 동기화합니다.
    // (공격 중 콤보 입력 플래그만 세운 경우는 제외 — ProcessComboCommand에서 처리)
    if (!bWasAttacking && HasStateTag("State.Action.Attacking"))
    {
        if (!HasStateTag("State.Movement.InAir"))
        {
            // 지상 콤보 1번째 공격 — 스냅 방향을 함께 전달합니다.
            MulticastPlayComboMontage(CombatComp->CurrentCombo, SnapRotation);
        }
        else
        {
            // 공중 점프 공격 Start 섹션
            MulticastPlayJumpAttackMontage();
        }
    }
}

// 모든 클라이언트에서 실행: 해당 콤보 단계의 몽타주를 재생합니다.
// Listen Server Host는 CombatComp->Attack()에서 이미 재생했으므로 스킵합니다.
// State.Action.Attacking 태그를 직접 추가해 CombatComponent::TickComponent()의 타겟 방향 회전 추적을 활성화합니다.
// (ActionTags는 복제되지 않으므로 Multicast 수신 측에서 직접 설정합니다.)
void AMyCharacter::MulticastPlayComboMontage_Implementation(int32 ComboIndex, FRotator SnapRotation)
{
    if (HasAuthority()) return;
    if (!CombatComp) return;

    // 서버로부터 전달받은 스냅 방향을 모든 클라이언트에 즉시 적용합니다.
    // IsLocallyControlled() 여부와 무관하게 적용해 모든 화면에서 방향이 동기화됩니다.
    SetActorRotation(SnapRotation);
    AddStateTag("State.Action.Attacking");
    CombatComp->CombatTargetActor = TargetingComp ? TargetingComp->CurrentTarget : nullptr;
    CombatComp->PlayComboMontageOnly(ComboIndex);
}

// 모든 클라이언트에서 실행: 점프 공격 몽타주(Start 섹션)를 재생합니다.
// Listen Server Host는 CombatComp->Attack()에서 이미 재생했으므로 스킵합니다.
void AMyCharacter::MulticastPlayJumpAttackMontage_Implementation()
{
    if (HasAuthority()) return;
    if (!CombatComp) return;
    CombatComp->PlayJumpAttackAnim();
}

// 모든 클라이언트에서 실행: 점프 공격 몽타주를 Loop 섹션으로 전환합니다.
// Listen Server Host는 CombatComp->ProcessComboCommand()에서 이미 처리했으므로 스킵합니다.
void AMyCharacter::MulticastPlayJumpLoopAnim_Implementation()
{
    if (HasAuthority()) return;
    if (!CombatComp) return;
    CombatComp->PlayJumpLoopAnim();
}

// 모든 클라이언트에서 실행: 착지 시 점프 공격 몽타주를 End 섹션으로 전환하거나 중단합니다.
// Listen Server Host는 Landed()에서 이미 처리했으므로 스킵합니다.
void AMyCharacter::MulticastPlayJumpLandingAnim_Implementation()
{
    if (HasAuthority()) return;
    if (!CombatComp) return;
    CombatComp->PlayJumpLandingAnim();
}

// 애니메이션 몽타주 중간에 발생하는 콤보 입력을 CombatComponent로 전달합니다.
// 서버에서 콤보가 실제로 진행된 경우 모든 클라이언트에 다음 단계 애님을 동기화합니다.
void AMyCharacter::ProcessComboCommand()
{
    if (!CombatComp) return;

    // 진행 여부 감지를 위해 호출 전 상태를 기록합니다.
    int32 ComboBefore = CombatComp->CurrentCombo;
    bool bWasComboInput = CombatComp->bIsComboInputOn;

    CombatComp->ProcessComboCommand();

    if (!HasAuthority()) return;

    int32 ComboAfter = CombatComp->CurrentCombo;

    if (!HasStateTag("State.Movement.InAir"))
    {
        // 지상: 콤보 카운터가 증가했으면 다음 콤보 섹션을 동기화합니다.
        if (ComboAfter > ComboBefore)
        {
            // 서버 플레이어는 CombatTargetActor에서, 클라이언트 플레이어는 LastSnapRotation을 재사용합니다.
            FRotator SnapRot = GetActorRotation();
            if (IsValid(CombatComp->CombatTargetActor))
            {
                FVector ToTarget = CombatComp->CombatTargetActor->GetActorLocation() - GetActorLocation();
                ToTarget.Z = 0.f;
                if (!ToTarget.IsNearlyZero()) SnapRot = ToTarget.Rotation();
            }
            else
            {
                SnapRot = LastSnapRotation;
            }
            MulticastPlayComboMontage(ComboAfter, SnapRot);
        }
    }
    else
    {
        // 공중: 콤보 입력이 있었으면 Loop 섹션 전환을 동기화합니다.
        // (PlayJumpLoopAnim은 CurrentCombo를 바꾸지 않으므로 bWasComboInput으로 감지)
        if (bWasComboInput)
        {
            MulticastPlayJumpLoopAnim();
        }
    }
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

    // 서버에서 착지했을 때 클라이언트에도 Loop → End 전환을 동기화합니다.
    // (클라이언트는 SimulatedProxy라 Landed()가 신뢰성 있게 호출되지 않으므로 명시적으로 전송)
    if (HasAuthority())
    {
        MulticastPlayJumpLandingAnim();
    }
}

// 피격 시 구르기 무적 여부를 판단하고, 실제 피해를 CombatComponent에 전달해 HP를 감소시킵니다.
float AMyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    UE_LOG(LogTemp, Warning, TEXT("[DMG_DBG] TakeDamage | Actor: %s | Amount: %.1f | Auth: %s"),
        *GetName(), DamageAmount, HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 피해 타입이 '회피 불가(UndodgeableDamageType)'인지 확인
    TSubclassOf<UDamageType> DamageTypeClass = DamageEvent.DamageTypeClass;
    bool bIsUndodgeable = (DamageTypeClass == UUndodgeableDamageType::StaticClass());

    // 구르기 중이고 회피 불가 피해가 아니라면 무적 처리
    if (HasStateTag("State.Action.Rolling") && !bIsUndodgeable)
    {
        return 0.0f;
    }

    // 실제 피해 적용 — CurrentHP가 Replicated이므로 OnRep_Stats가 클라이언트 HUD를 자동 갱신합니다.
    if (ActualDamage > 0.0f && CombatComp != nullptr)
    {
        EnterCombatStance();
        CombatComp->ReceiveDamage(ActualDamage);
    }

    return ActualDamage;
}

// Shift 키를 누르면 전투 자세면 구르기(8방향)를, 일반 상태면 달리기를 시작합니다.
void AMyCharacter::StartSprint()
{
    if (HasStateTag("State.Movement.InAir") || HasStateTag("State.Action.Attacking") || HasStateTag("State.Action.Rolling") || HasStateTag("State.Action.MagicCasting")) return;

    if (HasStateTag("State.Stance.Combat"))
    {
        float RollCost = 20.0f;
        if (!CombatComp || CombatComp->CurrentSP < RollCost) return;

        // 입력 방향(가속도)과 캐릭터 전방·우측 벡터의 내적으로 8방향 섹션 결정
        FVector Accel = GetCharacterMovement()->GetCurrentAcceleration();
        FName SectionName = FName("Backward");

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

        // 로컬에서 즉시 상태 태그와 몽타주를 반영해 반응성을 확보합니다.
        // Multicast가 도착하기 전에 Shift를 다시 누르는 경우를 방지합니다.
        AddStateTag("State.Action.Rolling");
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance && CombatRollMontage)
        {
            AnimInstance->Montage_Play(CombatRollMontage);
            AnimInstance->Montage_JumpToSection(SectionName, CombatRollMontage);
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &AMyCharacter::OnRollMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, CombatRollMontage);
        }

        // 클라이언트에서 계산한 방향을 서버로 전달합니다.
        ServerRequestRoll(SectionName);
        return;
    }

    // 일반 상태: 스태미나가 남아 있으면 이동 속도를 높여 달리기 시작 (소모는 Tick에서 처리)
    if (CombatComp && CombatComp->CurrentSP > 0.0f)
    {
        bIsSprinting = true;
        GetCharacterMovement()->MaxWalkSpeed = 800.0f;
        // 서버에도 속도 변경을 요청합니다. 서버가 같은 속도를 사용해야 위치 보정이 발생하지 않습니다.
        ServerStartSprint();
    }
}

// Shift 키를 떼면 이동 속도를 기본값으로 복구하고 달리기 플래그를 해제합니다.
void AMyCharacter::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = 500.0f;
    // 서버에도 속도 복구를 요청합니다.
    ServerStopSprint();
}

// 모든 클라이언트에서 실행: SP 소진 등으로 서버가 달리기를 강제 중단할 때 bIsSprinting과 이동 속도를 동기화합니다.
void AMyCharacter::MulticastForceStopSprint_Implementation()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = 500.0f;
}

// 서버에서 실행: MaxWalkSpeed를 달리기 속도로 변경합니다.
void AMyCharacter::ServerStartSprint_Implementation()
{
    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = 800.0f;
}

// 서버에서 실행: MaxWalkSpeed를 기본 이동 속도로 복구합니다.
void AMyCharacter::ServerStopSprint_Implementation()
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

    // 서버에서만 복제 변수를 설정합니다. 변경 시 OnRep_CombatStance()가 모든 클라이언트에서 자동 호출됩니다.
    if (HasAuthority() && !bIsInCombatStance)
    {
        bIsInCombatStance = true;
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

    // 서버에서만 복제 변수를 해제합니다.
    if (HasAuthority())
    {
        bIsInCombatStance = false;
    }
}

// bIsInCombatStance 복제 수신 시 호출 — 이동 방향 플래그와 상태 태그를 로컬에 모두 적용합니다.
// AnimBP가 bOrientRotationToMovement, State.Stance.Combat 태그, bIsInCombatStance 중 어느 것을 읽더라도 올바르게 동작합니다.
void AMyCharacter::OnRep_CombatStance()
{
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = !bIsInCombatStance;
        bUseControllerRotationYaw = bIsInCombatStance;
    }

    if (bIsInCombatStance)
    {
        AddStateTag("State.Stance.Combat");
    }
    else
    {
        RemoveStateTag("State.Stance.Combat");
    }
}

// 서버에서 실행: 스태미나 소모 및 구르기 로직을 처리한 뒤 모든 클라이언트에 애님을 동기화합니다.
void AMyCharacter::ServerRequestRoll_Implementation(FName SectionName)
{
    if (!CombatComp) return;

    float RollCost = 20.0f;
    if (CombatComp->CurrentSP < RollCost) return;

    CombatComp->CurrentSP -= RollCost;

    // 로컬 조종 캐릭터(호스트)는 StartSprint()에서 이미 재생했으므로 생략합니다.
    // 다시 재생하면 M1이 중단되면서 EndDelegate가 발동해 Rolling 태그가 조기 제거됩니다.
    if (!IsLocallyControlled())
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (!AnimInstance || !CombatRollMontage) return;

        AddStateTag("State.Action.Rolling");

        AnimInstance->Montage_Play(CombatRollMontage);
        AnimInstance->Montage_JumpToSection(SectionName, CombatRollMontage);

        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AMyCharacter::OnRollMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, CombatRollMontage);
    }

    MulticastPlayRollMontage(SectionName);
}

// 모든 클라이언트에서 실행: 다른 플레이어 화면(SimulatedProxy)에 구르기 몽타주를 재생합니다.
// Listen Server Host는 ServerRequestRoll에서 이미 재생했으므로 스킵합니다.
// 로컬 클라이언트는 StartSprint()에서 이미 즉시 재생했으므로 스킵합니다.
void AMyCharacter::MulticastPlayRollMontage_Implementation(FName SectionName)
{
    if (HasAuthority()) return;
    if (IsLocallyControlled()) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance || !CombatRollMontage) return;

    AddStateTag("State.Action.Rolling");

    AnimInstance->Montage_Play(CombatRollMontage);
    AnimInstance->Montage_JumpToSection(SectionName, CombatRollMontage);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AMyCharacter::OnRollMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, CombatRollMontage);
}

// 구르기 몽타주가 끝나면 구르기 상태 태그를 제거해 정상 상태로 복귀시킵니다.
void AMyCharacter::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    RemoveStateTag("State.Action.Rolling");
}

// 서버에서 실행: 클라이언트가 전달한 히트 위치에 이펙트를 스폰합니다.
// 공중 공격 시 서버의 SimulatedProxy 위치 오차로 WeaponTrace가 빗나가는 경우를 보정합니다.
void AMyCharacter::ServerReportWeaponHit_Implementation(FVector HitLocation, FRotator HitNormal)
{
    if (!CombatComp) return;
    CombatComp->SpawnHitEffectAt(HitLocation, HitNormal);
}

// 우클릭 마법 공격 입력 시 로컬 타겟을 파라미터로 담아 서버에 마법 공격을 요청합니다.
void AMyCharacter::MagicAttack()
{
    if (CombatComp)
    {
        if (HasStateTag("State.Action.Rolling")) return;
        AActor* LocalTarget = TargetingComp ? TargetingComp->CurrentTarget : nullptr;
        if (!LocalTarget) return;
        EnterCombatStance();
        ServerRequestMagicAttack(LocalTarget);
    }
}

// 서버에서 실행: 클라이언트가 전달한 타겟을 주입한 뒤 마법 콤보 로직을 처리하고 클라이언트에 동기화합니다.
void AMyCharacter::ServerRequestMagicAttack_Implementation(AActor* TargetActor)
{
    if (!CombatComp) return;

    // 클라이언트의 로컬 타겟을 서버의 TargetingComp에 주입합니다.
    // (TargetingComponent는 로컬 전용으로 동작하므로 서버 측 CurrentTarget이 비어 있습니다.)
    if (TargetingComp)
    {
        TargetingComp->CurrentTarget = TargetActor;
    }

    // 마법 시전 시 전투 자세로 전환합니다. bIsInCombatStance가 복제되어 다른 클라이언트에 스트레이프 애니메이션을 활성화합니다.
    EnterCombatStance();

    // 마법 시전 시작 전 상태를 기록해 실제로 새 마법이 시작됐는지 판단합니다.
    bool bWasCasting = HasStateTag("State.Action.MagicCasting");

    // 기존 마법 공격 로직 전체를 서버에서 실행합니다.
    // (상태 체크, 마나 소모, 콤보 카운터, 상태태그, 몽타주 재생 포함)
    CombatComp->RightClickMagicAttack();

    // 새 마법이 실제로 시작된 경우에만 Multicast로 클라이언트에 동기화합니다.
    // TargetActor의 위치를 함께 전달해 클라이언트에서도 VFX를 올바른 위치에 스폰할 수 있게 합니다.
    if (!bWasCasting && HasStateTag("State.Action.MagicCasting"))
    {
        MulticastPlayMagicMontage(CombatComp->CurrentMagicCombo, TargetActor->GetActorLocation());
    }
}

// 모든 클라이언트에서 실행: 해당 콤보 단계의 마법 몽타주를 재생합니다.
// TargetLocation을 CombatComponent에 미리 전달해 VFX가 올바른 위치에 스폰되도록 합니다.
// Listen Server Host는 RightClickMagicAttack()에서 이미 재생했으므로 스킵합니다.
void AMyCharacter::MulticastPlayMagicMontage_Implementation(int32 ComboIndex, FVector TargetLocation)
{
    if (HasAuthority()) return;
    if (!CombatComp) return;
    CombatComp->PlayMagicMontageOnly(ComboIndex, TargetLocation);
}

// 서버에서 실행: 아이템 줍기 요청을 검증하고 인벤토리에 추가한 뒤 액터를 제거합니다.
void AMyCharacter::ServerPickItem_Implementation(APickableItem* Item)
{
    if (!InventoryComp || !IsValid(Item)) return;

    // 아이템 데이터가 유효한지 확인합니다.
    if (Item->RuntimeItemData.Quantity <= 0 || Item->RuntimeItemData.ItemIcon == nullptr) return;

    // 인벤토리에 추가를 시도합니다. 실패(인벤토리 가득 참)하면 아이템을 유지합니다.
    bool bSuccess = InventoryComp->AddItemInternal(Item->RuntimeItemData);
    if (!bSuccess) return;

    // 획득 성공 시 월드에서 아이템 액터를 제거합니다.
    Item->Destroy();
}

// 서버에서 실행: 골드 잔액을 검증·차감하고 성공 시 인벤토리에 아이템을 추가합니다.
void AMyCharacter::ServerBuyItem_Implementation(FItemData Item)
{
    if (!InventoryComp || !MoneyComp) return;

    // 서버에서 골드 잔액을 검증하고 차감합니다. 잔액 부족 시 구매를 거부합니다.
    if (!MoneyComp->PayGoldInternal(Item.ItemPrice))
    {
        UE_LOG(LogTemp, Warning, TEXT("[MONEY_SYNC][SERVER][%s] ServerBuyItem rejected: insufficient gold for %s (cost: %d)"),
            *GetName(), *Item.ItemName, Item.ItemPrice);
        return;
    }

    InventoryComp->AddItemInternal(Item);
}

// 서버에서 실행: 퀘스트 보상 아이템을 인벤토리에 추가합니다.
void AMyCharacter::ServerClaimQuestReward_Implementation(FItemData Item)
{
    if (!InventoryComp) return;
    InventoryComp->AddItemInternal(Item);
}

// 서버에서 실행: MP 차감을 검증하고 스킬 애니메이션을 재생합니다.
// 투사체 생성은 서버에서 재생된 애니메이션의 AnimNotify_FireProjectile이 처리합니다.
void AMyCharacter::ServerCastSkill_Implementation(FName SkillID)
{
    if (!CombatComp || !SkillComp) return;

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
    if (!SkillSys) return;

    FSkillData* Data = SkillSys->GetSkillData(SkillID);
    if (!Data) { UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][1] %s -> SkillData NOT FOUND for ID: %s"), *GetName(), *SkillID.ToString()); return; }

    UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][1] %s -> ServerCastSkill called | SkillID: %s | Montage: %s"),
        *GetName(), *SkillID.ToString(), Data->SkillMontage ? *Data->SkillMontage->GetName() : TEXT("NULL"));

    // 서버에서 MP 잔액 검증 후 차감합니다.
    if (CombatComp->CurrentMP < Data->ManaCost) { UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][1] %s -> MP insufficient"), *GetName()); return; }
    CombatComp->CurrentMP -= Data->ManaCost;

    // 서버에서 애니메이션을 재생합니다. AnimNotify_FireProjectile이 서버에서 발동해 투사체를 생성합니다.
    if (Data->SkillMontage)
    {
        float PlayResult = PlayAnimMontage(Data->SkillMontage);
        UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][1] %s -> PlayAnimMontage result: %f (0 = failed)"), *GetName(), PlayResult);
        // 모든 클라이언트 화면에 스킬 애니메이션을 동기화합니다.
        MulticastPlaySkillMontage(Data->SkillMontage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[SKILL_DBG][1] %s -> SkillMontage is NULL, skipping"), *GetName());
    }

    // 버프 스킬은 서버에서 직접 스탯에 적용합니다.
    if (Data->SkillType == ESkillType::Buff && Data->BuffAmount > 0.0f)
    {
        CombatComp->ApplyAttackBuff(Data->BuffAmount, Data->Duration, SkillID);
    }
}

// 서버 → 시전자 클라이언트: 스킬 피격 데미지 텍스트를 시전자 화면에만 표시합니다.
void AMyCharacter::ClientShowDamageText_Implementation(FVector Location, float Damage)
{
    OnSpawnDamageText(Location, Damage);
}

// 서버 → 모든 클라이언트: 스킬 몽타주를 재생합니다.
// 시전자 본인은 TryCastSkill에서 이미 로컬 재생했으므로 건너뜁니다.
void AMyCharacter::MulticastPlaySkillMontage_Implementation(UAnimMontage* Montage)
{
    if (IsLocallyControlled()) return;
    if (Montage)
    {
        PlayAnimMontage(Montage);
    }
}
