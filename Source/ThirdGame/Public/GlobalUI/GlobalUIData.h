#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GlobalUIData.generated.h"

class UTexture2D;
class USoundBase;

UCLASS()
class THIRDGAME_API UGlobalUIData : public UDataAsset
{
    GENERATED_BODY()

public:
    // 전역으로 쓰일 골드 아이콘
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources|Image")
    UTexture2D* GoldIcon;

   
    // 서브시스템이 화면에 띄울 경고창 위젯의 설계도(클래스)입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources|UI")
    TSubclassOf<class UWarningWidget> WarningWidgetClass;
    
};