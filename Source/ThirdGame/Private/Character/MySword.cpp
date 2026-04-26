// =========================================================================================
// MySword.cpp
//
// [파일 역할]
// 플레이어 캐릭터가 장착하는 검(무기) 액터 클래스입니다.
// 실제 무기 충돌 판정은 CombatComponent의 WeaponTrace(SweepMultiByChannel)가 담당하므로
// 현재 이 클래스에는 별도 로직이 없습니다.
// =========================================================================================

#include "MySword.h"
#include "Enemy.h"
#include "Engine/Engine.h"
#include "MyCharacter.h"
#include "CombatComponent.h"
#include "Kismet/GameplayStatics.h"

// Tick이 불필요하므로 비활성화합니다.
AMySword::AMySword()
{
	PrimaryActorTick.bCanEverTick = false;
}

// 현재 사용하지 않습니다.
void AMySword::BeginPlay()
{
	Super::BeginPlay();
}

// 현재 사용하지 않습니다.
void AMySword::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 현재 사용하지 않습니다. 무기 충돌 판정은 CombatComponent::WeaponTraceTick에서 처리합니다.
void AMySword::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
}
