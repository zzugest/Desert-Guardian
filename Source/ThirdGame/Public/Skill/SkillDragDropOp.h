#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "SkillDragDropOp.generated.h"

/**
 * 스킬 드래그 앤 드롭 전용 택배 상자
 */
UCLASS()
class THIRDGAME_API USkillDragDropOp : public UDragDropOperation
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Skill Drag Drop")
	FName SkillID;
};