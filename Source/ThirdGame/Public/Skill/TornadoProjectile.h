#pragma once

#include "CoreMinimal.h"
#include "ProjectileBase.h"
#include "TornadoProjectile.generated.h"

UCLASS()
class THIRDGAME_API ATornadoProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	ATornadoProjectile();

protected:
	virtual void BeginPlay() override;

	// 0.5초마다 실행될 다단 히트 함수
	UFUNCTION()
	void ApplyPeriodicDamage();

	// 타이머를 껐다 켤 수 있게 관리해주는 핸들
	FTimerHandle DamageTimerHandle;

	// 데미지를 주는 간격
	UPROPERTY(EditAnywhere, Category = "Damage")
	float DamageInterval = 0.5f;
};