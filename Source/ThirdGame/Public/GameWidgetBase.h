#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameWidgetBase.generated.h"

UCLASS()
class THIRDGAME_API UGameWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	// 에디터에서 설정할 수 있는 옵션
	// 체크하면: 움직임 제한 (상점, 대화창 등)
	// 체크 해제하면: 움직임 허용 (인벤토리, 미니맵 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Option")
	bool bShouldBlockMoveInput = true; // 기본값은 '제한함'으로 설정
};