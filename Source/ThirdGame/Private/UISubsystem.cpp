// =========================================================================================
// UISubsystem.cpp
//
// [���� ���]
// ���� �� ���� UI�� ����/���� ���¸� �߾� ���߽����� �����Ͽ� ���콺 Ŀ�� ǥ�� �� �÷��̾� �Է� ��带 �ϰ� �����ϴ� ������ ����ý����Դϴ�.
// =========================================================================================

#include "UISubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameWidgetBase.h"
#include "Framework/Application/SlateApplication.h"

// ���ο� UI�� ȭ�鿡 ������� �� ���� ��Ͽ� ����ϰ� �Է� ��带 �����մϴ�.
void UUISubsystem::ReportUIOpened(UUserWidget* Widget)
{
    if (!Widget) return;

    // ������ ������ ���� �� �ߺ� ��ϵǾ� ���� ������ ���̴� ���� ��õ �����մϴ�.
    OpenWidgets.AddUnique(Widget);
    UpdateInputMode();
}

// Ȱ��ȭ�Ǿ��� Ư�� UI�� ������ �� ���� ��Ͽ��� ���ܽ�Ű�� �Է� ��带 ���� �����մϴ�.
void UUISubsystem::ReportUIClosed(UUserWidget* Widget)
{
    if (!Widget) return;

    OpenWidgets.Remove(Widget);
    UpdateInputMode();
}

// ���� ����� UI Ư���� �����Ͽ� ����/UI �Է� ����ǰ� ���콺 Ŀ���� Ȱ��ȭ ���¸� �����մϴ�.
void UUISubsystem::UpdateInputMode()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // �����ִ� UI�� �ϳ��� ���ٸ� ���콺�� ����� ������ ���� ���� ���·� ��� �ǵ����ϴ�.
    if (OpenWidgets.IsEmpty())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(false);
        PC->SetIgnoreMoveInput(false);
        return;
    }

    bool bNeedBlockMove = false;

    // ��ϵ� ���� �� �� �ϳ��� ĳ���� �̵� ���� ������ �䱸�ϴ��� �Ǻ��մϴ�.
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

    // �Է� ��Ŀ���� ������ ������ Ƣ�� �ʵ��� ���� ���߿� ����(�ֻ��) UI�� ���� ����
    if (OpenWidgets.Last())
    {
        InputMode.SetWidgetToFocus(OpenWidgets.Last()->TakeWidget());
    }

    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(true);

    // �����̳� ����Ʈ ��ȭ �� �Ǽ��� ĳ���Ͱ� �̵��� ������ �һ�縦 �����ϱ� ����
    PC->ResetIgnoreMoveInput();

    if (bNeedBlockMove)
    {
        PC->SetIgnoreMoveInput(true); // ���ƾ� �� ���� ����ϰ� +1
    }
    else
    {
        // ���� �̾ư� �� Ű���� ��Ŀ���� ������ ���� ȭ��(Viewport)���� �ǵ��� �����ϴ�!
        FSlateApplication::Get().SetAllUserFocusToGameViewport();
    }
}

// ���� ����, �� ��ȯ �� ������ ȭ���� �����ؾ� �� �� ������ �˾� UI ��ü�� �ϰ� �Ҹ��ŵ�ϴ�.
void UUISubsystem::CloseAllActiveUIs()
{
    if (OpenWidgets.IsEmpty()) return;

    // ������ ��ȸ �� ��� ������ ���� �迭 �ε����� �з� �߻��ϴ� ũ���ø� ������ �������� �����ϰ� �����մϴ�.
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