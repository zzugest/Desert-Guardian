#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "EliteMonster.generated.h"

UCLASS()
class THIRDGAME_API AEliteMonster : public AEnemy
{
	GENERATED_BODY()

public:
	AEliteMonster();

protected:
	virtual void BeginPlay() override;

public:
	virtual void ExecuteAttackHit() override;

	UPROPERTY()
	ACharacter* TargetPlayer;
};
