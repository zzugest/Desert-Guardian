#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WarningSubsystem.generated.h"

class UWarningWidget;

UCLASS()
class THIRDGAME_API UWarningSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 게임 어디서든 이 함수만 부르면 경고창이 뜹니다!
	UFUNCTION(BlueprintCallable, Category = "Warning System")
	void ShowWarning(FText Message);

private:
	// 화면에 딱 하나만 띄워둘 경고 위젯을 기억하는 변수입니다.
	UPROPERTY()
	UWarningWidget* CurrentWarningWidget;
};