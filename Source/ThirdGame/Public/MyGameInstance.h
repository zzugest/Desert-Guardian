#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

class UGlobalUIData; // 우리가 만든 데이터 에셋을 알기 위한 전방 선언

UCLASS()
class THIRDGAME_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// 에디터에서 전역 데이터 에셋(DA_GlobalUI)을 꽂아줄 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Data")
	UGlobalUIData* GlobalUIData;
};