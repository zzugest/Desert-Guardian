// =========================================================================================
// EnemyAnimInstance.cpp
//
// [파일 역할]
// AEnemy에 사용되는 적 캐릭터 전용 애니메이션 인스턴스 컨트롤러 베이스입니다.
// 속도, 이동 중 여부, 공중/지상 상태, 그리고 사망 여부를 판단할 수 있는 변수 데이터를 제공합니다.
// =========================================================================================

#include "EnemyAnimInstance.h"
#include "Enemy.h"

// 애니메이션 초기화 시 소유 적 액터를 캐시에 저장합니다.
void UEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerEnemy = Cast<AEnemy>(TryGetPawnOwner());
}

// 매 프레임 이동 속도와 사망 여부를 갱신하여 애니메이션 상태 파라미터에 반영합니다.
void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerEnemy) return;

	// Z축 속도는 제외하고 수평 이동 속도만 계산합니다.
	FVector Velocity = OwnerEnemy->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();

	// HP가 0 이하이면 사망 상태로 판단해 AnimBP에서 사망 애니메이션을 재생합니다.
	bIsDead = OwnerEnemy->CurrentHP <= 0.0f;
}
