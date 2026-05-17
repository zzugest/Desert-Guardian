#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h" 
#include "TutorialDummy.generated.h"

UCLASS()
class THIRDGAME_API ATutorialDummy : public AEnemy
{
	GENERATED_BODY()

public:
	ATutorialDummy();

protected:
	virtual void BeginPlay() override;

	// 데미지를 무한으로 흡수하기 위해 부모의 TakeDamage를 덮어씁니다!
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
};