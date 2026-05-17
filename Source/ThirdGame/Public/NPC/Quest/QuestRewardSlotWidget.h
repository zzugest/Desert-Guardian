#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestRewardSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

UCLASS()
class THIRDGAME_API UQuestRewardSlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 아이템 이미지를 띄울 이미지 컴포넌트
    UPROPERTY(meta = (BindWidget))
    UImage* RewardIcon;

    // 아이템 이름과 개수를 띄울 텍스트 컴포넌트
    UPROPERTY(meta = (BindWidget))
    UTextBlock* RewardText;

    // 대화창에서 이 함수를 불러서 데이터를 채워넣을 겁니다.
    void SetupSlot(UTexture2D* Icon, FString Name, int32 Quantity);
};