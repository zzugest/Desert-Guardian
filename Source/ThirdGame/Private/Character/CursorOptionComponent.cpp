// =========================================================================================
// CursorOptionComponent.cpp
//
// [파일 역할]
// 플레이어 캐릭터에 붙어 Ctrl 키 기반 마우스 커서 표시/숨김을 담당하는 컴포넌트입니다.
// - Ctrl 누름: GameAndUI 입력 모드 전환 + 커서 표시
// - Ctrl 해제: GameOnly 입력 모드 복원 + 커서 숨김
// - UI 버튼 클릭 시 Enhanced Input이 Ctrl Completed를 잘못 소모하는 경우를 대비해
//   Tick에서 OS 레벨 키 상태를 폴링하는 fallback을 제공합니다.
// - UISubsystem이 관리하는 UI(포탈 확인창, 대화창 등)가 열려있을 때는 커서를 끄지 않습니다.
// =========================================================================================

#include "Character/CursorOptionComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Framework/Application/SlateApplication.h"
#include "UISubsystem.h"
#include "Kismet/GameplayStatics.h"

UCursorOptionComponent::UCursorOptionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCursorOptionComponent::BeginPlay()
{
	Super::BeginPlay();
}

// MyCharacter::SetupPlayerInputComponent에서 호출해 Ctrl 커서 액션을 바인딩합니다.
void UCursorOptionComponent::BindInputActions(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (!EnhancedInputComponent) return;

	if (ShowCursorAction)
	{
		EnhancedInputComponent->BindAction(ShowCursorAction, ETriggerEvent::Started,   this, &UCursorOptionComponent::OnShowCursorPressed);
		EnhancedInputComponent->BindAction(ShowCursorAction, ETriggerEvent::Completed, this, &UCursorOptionComponent::OnShowCursorReleased);
	}
}

// Ctrl 키를 누를 때 마우스 커서를 표시하고 게임+UI 복합 입력 모드로 전환합니다.
void UCursorOptionComponent::OnShowCursorPressed(const FInputActionValue& Value)
{
	// UISubsystem이 이미 UI를 열어 커서를 관리 중이면 Ctrl은 아무 작업도 하지 않습니다.
	UGameInstance* GI    = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UUISubsystem*  UISys = GI ? GI->GetSubsystem<UUISubsystem>() : nullptr;
	if (UISys && !UISys->HasNoOpenWidgets()) return;

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	APlayerController* PC = OwnerChar ? Cast<APlayerController>(OwnerChar->GetController()) : nullptr;
	if (!PC) return;

	UE_LOG(LogTemp, Warning, TEXT("[Cursor] Pressed - 커서 ON"));
	bCtrlCursorActive = true;
	PC->bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
}

// Ctrl 키를 뗄 때 마우스 커서를 숨기고 게임 전용 입력 모드로 복원합니다.
void UCursorOptionComponent::OnShowCursorReleased(const FInputActionValue& Value)
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	APlayerController* PC = OwnerChar ? Cast<APlayerController>(OwnerChar->GetController()) : nullptr;
	if (!PC) return;

	// 클릭 등으로 인해 Enhanced Input이 Ctrl을 해제로 오인하는 경우를 방지합니다.
	// OS 레벨 키 상태로 확인해 Ctrl이 실제로 눌려있으면 무시합니다.
	if (FSlateApplication::Get().GetModifierKeys().IsControlDown())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cursor] Released 무시 - Ctrl 아직 눌려있음"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Cursor] Released - 커서 OFF"));
	bCtrlCursorActive = false;
	PC->bShowMouseCursor = false;
	PC->SetInputMode(FInputModeGameOnly());
}

void UCursorOptionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ── Ctrl 커서 fallback ───────────────────────────────────────────────────
	// UI 버튼 클릭 시 Enhanced Input이 Ctrl Completed를 잘못 소모해 실제 해제를 감지 못하는 경우를 대비합니다.
	// bCtrlCursorActive가 true인 동안만 동작하므로 포탈·대화창 등 UISubsystem 관리 커서와 충돌하지 않습니다.
	if (!bCtrlCursorActive) return;
	if (FSlateApplication::Get().GetModifierKeys().IsControlDown()) return;

	ACharacter* OwnerChar  = Cast<ACharacter>(GetOwner());
	APlayerController* PC  = OwnerChar ? Cast<APlayerController>(OwnerChar->GetController()) : nullptr;
	UGameInstance* GI      = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UUISubsystem*  UISys   = GI ? GI->GetSubsystem<UUISubsystem>() : nullptr;

	bCtrlCursorActive = false;

	// UISubsystem이 관리하는 UI가 없을 때만 커서를 끕니다.
	// 포탈 확인창·대화창 등이 열려있으면 UISubsystem이 커서를 계속 관리하도록 둡니다.
	if (PC && (!UISys || UISys->HasNoOpenWidgets()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cursor] Tick fallback - 커서 OFF"));
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}
