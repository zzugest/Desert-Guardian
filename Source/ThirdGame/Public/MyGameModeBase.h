#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

UCLASS()
class THIRDGAME_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	// 블루프린트에서 마법, 보스 공격 등 미리 로딩할 이펙트들을 마음껏 넣을 수 있는 배열!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Preload")
	TArray<class UNiagaraSystem*> PreloadVFXList;
};