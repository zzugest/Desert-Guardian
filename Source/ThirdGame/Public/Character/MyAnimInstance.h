// =========================================================================================
// MyAnimInstance.h
//
// [역할 요약]
// 플레이어 캐릭터의 애니메이션 블루프린트를 구동시키기 위한 변수(속도, 점프, 회전 기울기 등)를 계산하고 조달하는 중간 관리자 헤더입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MyAnimInstance.generated.h"

UCLASS()
class THIRDGAME_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UMyAnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 캐스팅 비용 절감을 위해 현재 애니메이션을 소유한 플레이어 객체를 캐싱해 둡니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class AMyCharacter* OwnerCharacter;

	// 실시간 낙하 및 가속 정보를 빠르게 얻기 위해 무브먼트 객체를 캐싱해 둡니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCharacterMovementComponent* MovementComponent;

	// 걷기, 달리기 블렌드 연산에 쓰일 현재 수평 이동 속력입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Speed;

	// 현재 캐릭터의 발이 공중에 떠있는지 판단하는 상태값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsInAir;

	// 외부 힘이나 사용자 입력에 의해 속도가 붙고 있는 중인지 반환합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsAccelerating;

	// 캐릭터 상체(Aim Offset) 회전 각도값들입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Pitch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Roll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Yaw;

	// 방향 전환(Lean) 자연성을 위해 이전 프레임 대비 몸통의 요동(변화량)을 측정한 수치입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float YawDelta;

	// Delta 계산을 위해 지난 프레임에 기록해둔 캐릭터 각도값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator RotationLastTick;

	// 전신 애니메이션 몽타주를 강제로 덧씌울지 판별하는 플래그입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFullBody;

	// 달리기 여부 — Speed 관성 없이 즉시 AnimBP에 전달됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsSprinting;
};