// =========================================================================================
// WarningSubsystem.cpp
//
// [파일 역할]
// 화면 중앙에 경고 메시지(토스트 알림)를 표시하는 GameInstance 서브시스템입니다.
// WarningWidget을 최초 호출 시 한 번만 생성해 Z-Order 99(최상위)로 뷰포트에 추가하고,
// 이후 호출에서는 텍스트만 바꾸고 애니메이션을 재생합니다.
//
// [사용 예시]
// UWarningSubsystem* WarnSys = GameInstance->GetSubsystem<UWarningSubsystem>();
// WarnSys->ShowWarning(FText::FromString(TEXT("마나가 부족합니다.")));
// =========================================================================================

#include "GlobalUI/WarningSubsystem.h"
#include "GlobalUI/WarningWidget.h"
#include "MyGameInstance.h"
#include "GlobalUI/GlobalUIData.h"
#include "Blueprint/UserWidget.h"

// 경고 메시지를 화면에 표시합니다.
// 위젯이 없으면 GlobalUIData에서 클래스를 읽어 생성하고, 있으면 텍스트만 갱신합니다.
void UWarningSubsystem::ShowWarning(FText Message)
{
	// 위젯이 아직 없으면 GlobalUIData에서 클래스를 가져와 최초 생성합니다.
	if (!CurrentWarningWidget)
	{
		UMyGameInstance* GameInst = Cast<UMyGameInstance>(GetGameInstance());

		if (GameInst && GameInst->GlobalUIData && GameInst->GlobalUIData->WarningWidgetClass)
		{
			CurrentWarningWidget = CreateWidget<UWarningWidget>(GameInst, GameInst->GlobalUIData->WarningWidgetClass);
			if (CurrentWarningWidget)
			{
				// 다른 모든 UI 위에 겹치도록 Z-Order를 99(최상위)로 설정해 뷰포트에 추가합니다.
				CurrentWarningWidget->AddToViewport(99);
			}
		}
	}

	// 위젯에 새 텍스트를 전달하고 페이드 인/아웃 애니메이션을 재생합니다.
	if (CurrentWarningWidget)
	{
		CurrentWarningWidget->ShowWarningMessage(Message);
	}
}
