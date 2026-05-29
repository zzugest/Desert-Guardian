#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "CursorOptionComponent.generated.h"

class UInputAction;
class UEnhancedInputComponent;
class UUISubsystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class THIRDGAME_API UCursorOptionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCursorOptionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// MyCharacter의 SetupPlayerInputComponent에서 호출해 Ctrl 커서 액션을 바인딩합니다.
	void BindInputActions(UEnhancedInputComponent* EnhancedInputComponent);

	// Ctrl 키를 누를 때 마우스 커서를 표시하고 UI 입력을 허용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ShowCursorAction;

protected:
	virtual void BeginPlay() override;

private:
	// Ctrl 커서 모드 활성화 여부입니다. Enhanced Input 이벤트 소실 시 Tick fallback에 사용합니다.
	bool bCtrlCursorActive = false;

	// Ctrl 키를 누를 때 마우스 커서를 표시하고 게임+UI 복합 입력 모드로 전환합니다.
	void OnShowCursorPressed(const FInputActionValue& Value);

	// Ctrl 키를 뗄 때 마우스 커서를 숨기고 게임 전용 입력 모드로 복원합니다.
	void OnShowCursorReleased(const FInputActionValue& Value);
};
