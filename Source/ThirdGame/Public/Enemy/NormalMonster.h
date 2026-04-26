#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "NormalMonster.generated.h"

// ���� ����
class UPawnSensingComponent;
class ACharacter;

UCLASS()
class THIRDGAME_API ANormalMonster : public AEnemy
{
	GENERATED_BODY()

public:
	ANormalMonster();

protected:
	virtual void BeginPlay() override;

public:
	virtual void ExecuteAttackHit() override;

protected:
	bool bCanAttack = true;
	FTimerHandle AttackTimerHandle;

	UPROPERTY()
	ACharacter* TargetPlayer;
};
