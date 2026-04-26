#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillDragVisual.generated.h"

class UImage; // 전방 선언

UCLASS()
class THIRDGAME_API USkillDragVisual : public UUserWidget
{
    GENERATED_BODY()

public:
	// 외부에서 아이콘을 보여주는 함수: 스킬 드래그 시작 시 호출되어 아이콘을 설정
    void SetDragIcon(UTexture2D* IconTexture);

protected:
    // 위젯 디자이너에 있는 이미지와 연결
    UPROPERTY(meta = (BindWidget))
    UImage* DragIconImage;
};