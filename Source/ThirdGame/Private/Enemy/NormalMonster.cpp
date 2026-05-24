// =========================================================================================
// NormalMonster.cpp
//
// [파일 역할]
// AEnemy를 상속받는 일반 몬스터(잡몹) 클래스입니다.
// BeginPlay에서 블루프린트에 붙어있는 무기 스태틱 메시를 자동으로 탐색해 캐싱하고,
// 공격 시 해당 메시의 소켓을 기준으로 충돌 트레이스가 수행됩니다.
// =========================================================================================

#include "NormalMonster.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BlackboardComponent.h"

// 부모(AEnemy)의 기본값(Tick 활성화 등)을 유지합니다.
ANormalMonster::ANormalMonster()
{
	// AEnemy::Tick이 HP바 카메라 방향 빌보딩을 처리하므로 Tick이 필요합니다.
}

// WeaponMesh 탐색은 Enemy::BeginPlay로 이동해 모든 서브클래스에서 공통 처리합니다.
void ANormalMonster::BeginPlay()
{
	Super::BeginPlay();
}

// 현재 미사용. 실제 데미지 처리는 ANS_NormalEnemy_BaseAttackTrace가 담당합니다.
void ANormalMonster::ExecuteAttackHit()
{
}
