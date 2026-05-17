#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "QuestMarkerWidget.generated.h"

UCLASS()
class THIRDGAME_API UQuestMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 블루프린트의 텍스트 블록 이름과 똑같이 맞춰야 합니다! (예: MarkerText)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MarkerText;
};