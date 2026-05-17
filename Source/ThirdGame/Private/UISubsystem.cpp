// =========================================================================================
// UISubsystem.cpp
//
// [파일 역할]
// 게임 내 모든 UI의 열림/닫힘 상태를 중앙 서브시스템에서 관리하여 마우스 커서 표시 및
// 플레이어 입력 모드를 일괄 처리하는 월드 서브시스템입니다.
// =========================================================================================

#include "UISubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameWidgetBase.h"
#include "Framework/Application/SlateApplication.h"

// 새로운 UI가 화면에 표시될 때 열린 목록에 등록하고 입력 모드를 갱신합니다.
void UUISubsystem::ReportUIOpened(UUserWidget* Widget)
{
    if (!Widget) return;

    // 동일한 위젯이 중복 등록되어 있는 경우를 방지하는 안전성 처리를 합니다.
    OpenWidgets.AddUnique(Widget);
    UpdateInputMode();
}

// 활성화되어 있던 특정 UI가 닫힐 때 열린 목록에서 제외시키고 입력 모드를 다시 갱신합니다.
void UUISubsystem::ReportUIClosed(UUserWidget* Widget)
{
    if (!Widget) return;

    OpenWidgets.Remove(Widget);
    UpdateInputMode();
}

// 현재 열린 UI 특성에 따라 게임/UI 입력 전환과 마우스 커서의 활성화 상태를 처리합니다.
void UUISubsystem::UpdateInputMode()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // 열려있는 UI가 하나도 없다면 마우스 커서를 숨기고 게임 전용 입력 상태로 자동 복원합니다.
    if (OpenWidgets.IsEmpty())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(false);
        PC->SetIgnoreMoveInput(false);
        return;
    }

    bool bNeedBlockMove = false;

    // 등록된 위젯 중 하나라도 캐릭터 이동 입력 차단을 요구하는지 판별합니다.
    for (UUserWidget* Widget : OpenWidgets)
    {
        if (!Widget) continue;

        UGameWidgetBase* GameWidget = Cast<UGameWidgetBase>(Widget);
        if (GameWidget && GameWidget->bShouldBlockMoveInput)
        {
            bNeedBlockMove = true;
            break;
        }
    }

    FInputModeGameAndUI InputMode;

    // 입력 포커스가 다른 위젯으로 이전되지 않도록 현재 목록에서 가장 위(최상위) UI에 포커스를 설정합니다.
    if (OpenWidgets.Last())
    {
        InputMode.SetWidgetToFocus(OpenWidgets.Last()->TakeWidget());
    }

    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(true);

    // 레벨이나 레이아웃 변화 후 누적된 이동 차단 카운터를 초기화하여 오작동을 방지합니다.
    PC->ResetIgnoreMoveInput();

    if (bNeedBlockMove)
    {
        PC->SetIgnoreMoveInput(true); // 막아야 할 이동 입력이면 카운터 +1
    }
    else
    {
        // 이동이 허용될 때 키보드 포커스가 게임 화면(Viewport)으로 되돌아오도록 복구합니다.
        FSlateApplication::Get().SetAllUserFocusToGameViewport();
    }
}

// 게임 종료, 씬 전환 등 화면에 남아있는 모든 UI를 일괄 제거합니다.
void UUISubsystem::CloseAllActiveUIs()
{
    if (OpenWidgets.IsEmpty()) return;

    // 역순으로 순회하여 배열 인덱스 변동으로 인한 오류를 방지하면서 모든 위젯을 제거합니다.
    for (int32 i = OpenWidgets.Num() - 1; i >= 0; --i)
    {
        UUserWidget* Widget = OpenWidgets[i];
        if (Widget)
        {
            Widget->RemoveFromParent();
        }
    }

    OpenWidgets.Empty();
    UpdateInputMode();
}
