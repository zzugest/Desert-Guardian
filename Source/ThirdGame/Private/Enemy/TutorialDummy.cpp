// =========================================================================================
// TutorialDummy.cpp
//
// [파일 역할]
// 튜토리얼용 허수아비(샌드백) 적 클래스입니다.
// 피격되어도 사망하지 않고, AI 제어 없이 제자리에 고정되어 있습니다.
// 데미지를 받으면 HP가 즉시 최대치로 회복되어 플레이어가 전투 연습을 할 수 있습니다.
// =========================================================================================

#include "TutorialDummy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyHPBarWidget.h"
#include "Components/WidgetComponent.h"

// AI 없이 제자리에 고정되도록 AutoPossessAI를 비활성화합니다.
ATutorialDummy::ATutorialDummy()
{
	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;
}

// 이동 모드를 None으로 설정해 어디로도 이동하거나 밀리지 않게 고정합니다.
void ATutorialDummy::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->SetMovementMode(MOVE_None);
}

// 피격 시 사망하지 않고 HP를 즉시 최대치로 회복한 뒤 데미지 수치를 반환합니다.
// Super::TakeDamage를 호출하지 않아 부모(AEnemy)의 사망 처리를 우회합니다.
float ATutorialDummy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount <= 0.0f) return 0.0f;

	// 피격 즉시 HP를 최대치로 복구합니다.
	CurrentHP = MaxHP;

	// HP 바 UI를 만체 상태로 갱신합니다.
	if (HPBarWidget)
	{
		UEnemyHPBarWidget* EnemyWidget = Cast<UEnemyHPBarWidget>(HPBarWidget->GetUserWidgetObject());
		if (EnemyWidget) EnemyWidget->UpdateHPWidget(CurrentHP, MaxHP);
	}

	return DamageAmount;
}
