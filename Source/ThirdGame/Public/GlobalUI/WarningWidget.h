#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WarningWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class THIRDGAME_API UWarningWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 서브시스템이 이 함수를 불러서 메시지를 전달할 겁니다.
	UFUNCTION(BlueprintCallable, Category = "Warning UI")
	void ShowWarningMessage(FText NewMessage);

protected:
	//  [BindWidget] 블루프린트에서 똑같은 이름(Txt_WarningMessage)으로 텍스트를 만들면 C++과 자동 연결됩니다!
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_WarningMessage;

	//  [BindWidgetAnim] 블루프린트에서 똑같은 이름(Anim_FadeInOut)으로 애니메이션을 만들면 자동 연결됩니다!
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* Anim_FadeInOut;
};