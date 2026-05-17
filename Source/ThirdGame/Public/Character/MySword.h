// =========================================================================================
// MySword.h
//
// [역할 요약]
// 플레이어가 장착할 기본 근접 무기(검)의 베이스 액터 헤더입니다. 외형을 표시하는 스태틱 메시와 기본적인 오버랩 충돌 함수를 담고 있습니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MySword.generated.h"

UCLASS()
class THIRDGAME_API AMySword : public AActor
{
	GENERATED_BODY()

public:
	AMySword();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 플레이어가 착용하고 휘두를 검의 뼈대 외형(3D 모델) 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* SwordMesh;

	// 검날이 다른 액터의 콜리전 영역을 침범했을 때 검출되는 이벤트 콜백입니다.
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};