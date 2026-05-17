// =========================================================================================
// UISubsystem.h
//
// [역할 요약]
// 게임 내 여러 UI의 열림/닫힘 상태를 중앙 집중식으로 추적하여 마우스 커서 표시 제한 및 플레이어 입력 제어권을 일괄적으로 관리하는 서브시스템 헤더입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "UISubsystem.generated.h"

UCLASS()
class THIRDGAME_API UUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 새로운 UI가 화면에 띄워졌을 때 관리 목록에 등록하고 마우스/키보드 입력 모드를 갱신합니다.
	UFUNCTION(BlueprintCallable)
	void ReportUIOpened(UUserWidget* Widget);

	// 활성화되었던 특정 UI가 닫혔을 때 관리 목록에서 제외시키고 입력 모드를 원상 복구합니다.
	UFUNCTION(BlueprintCallable)
	void ReportUIClosed(UUserWidget* Widget);

	// 전투 진입, 씬 전환 등 강제로 화면을 정리해야 할 때 현재 띄워진 팝업 UI 전체를 일괄 소멸시킵니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseAllActiveUIs();

private:
	// 현재 화면에 렌더링되어 상호작용 가능한 상태로 열려있는 위젯들의 실시간 추적 목록입니다.
	UPROPERTY()
	TArray<UUserWidget*> OpenWidgets;

	// 활성화된 위젯들의 특성(이동 차단 여부 등)을 종합하여 게임/UI 입력 제어권과 마우스 커서의 활성화 상태를 조율합니다.
	void UpdateInputMode();
};