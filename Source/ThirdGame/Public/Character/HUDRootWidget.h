#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDRootWidget.generated.h"

class UPlayerHUDWidget;
class UMinimapWidget;
class UQuestLogWidget;

// =========================================================================================
// HUDRootWidget.h
//
// [역할 설명]
// 플레이어 HUD 전체를 하나의 루트 위젯으로 묶습니다.
// PlayerHUD, MinimapWidget, QuestLogWidget을 자식으로 가지며,
// 이 루트 하나만 숨기면 모든 HUD가 함께 사라집니다.
//
// [에디터 설정]
// WBP_HUDRoot Blueprint를 열고 아래 이름의 자식 위젯을 배치하세요.
//   - PlayerHUD       (WBP_PlayerHUD)
//   - MinimapWidget   (WBP_Minimap)
//   - QuestLogWidget  (WBP_QuestLog)
// =========================================================================================

UCLASS()
class THIRDGAME_API UHUDRootWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// WBP_HUDRoot 안에 이름이 "PlayerHUD"인 자식 위젯과 연결됩니다.
	UPROPERTY(meta = (BindWidget))
	UPlayerHUDWidget* PlayerHUD;

	// WBP_HUDRoot 안에 이름이 "MinimapWidget"인 자식 위젯과 연결됩니다.
	UPROPERTY(meta = (BindWidget))
	UMinimapWidget* MinimapWidget;

	// WBP_HUDRoot 안에 이름이 "QuestLogWidget"인 자식 위젯과 연결됩니다.
	UPROPERTY(meta = (BindWidget))
	UQuestLogWidget* QuestLogWidget;
};
