// =========================================================================================
// MyAnimInstance.cpp
//
// [파일 역할]
// 애니메이션 블루프린트(AnimBP)의 C++ 기반 클래스입니다.
// 캐릭터의 이동 속도, 공중 여부, 가속 여부, 회전 변화량(YawDelta) 등을
// 매 프레임 읽어 애니메이션 파라미터로 업데이트합니다.
// =========================================================================================

#include "MyAnimInstance.h"
#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// 모든 애니메이션 파라미터를 기본값으로 초기화합니다.
UMyAnimInstance::UMyAnimInstance()
{
	Speed = 0.0f;
	bIsInAir = false;
	bIsAccelerating = false;
	Pitch = 0.0f;
	Roll = 0.0f;
	Yaw = 0.0f;
	YawDelta = 0.0f;
	bFullBody = false;

	RotationLastTick = FRotator::ZeroRotator;
	MovementComponent = nullptr;
	OwnerCharacter = nullptr;
}

// 게임 시작 시 소유 캐릭터와 MovementComponent를 캐싱합니다.
void UMyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
	if (!OwnerCharacter) return;

	MovementComponent = OwnerCharacter->GetCharacterMovement();
}

// 매 프레임 캐릭터의 속도·가속·회전 정보를 읽어 AnimBP 파라미터를 최신 상태로 갱신합니다.
void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 소유 캐릭터가 아직 설정되지 않은 경우 재시도
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
		if (OwnerCharacter)
		{
			MovementComponent = OwnerCharacter->GetCharacterMovement();
		}
	}

	if (!OwnerCharacter) return;

	// 걷기·달리기 애니메이션 전환을 위해 XY 평면 이동 속도를 계산합니다.
	FVector Velocity = OwnerCharacter->GetVelocity();
	Speed = UKismetMathLibrary::VSizeXY(Velocity);

	// 공중 여부와 가속 여부를 MovementComponent에서 읽어 애니메이션 상태를 결정합니다.
	if (MovementComponent)
	{
		bIsInAir       = MovementComponent->IsFalling();
		bIsAccelerating = (MovementComponent->GetCurrentAcceleration().Size() > 0.0f);
	}
	else
	{
		bIsInAir       = false;
		bIsAccelerating = false;
	}

	// Aim Offset 계산을 위해 캐릭터의 현재 회전값을 읽습니다.
	FRotator CharacterRotation = OwnerCharacter->GetActorRotation();
	Pitch = CharacterRotation.Pitch;
	Roll  = CharacterRotation.Roll;
	Yaw   = CharacterRotation.Yaw;

	// 린(Lean) 애니메이션 처리를 위해 지난 프레임 대비 Yaw 변화량을 계산합니다.
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, RotationLastTick);
	float Target = 0.0f;

	if (DeltaSeconds > KINDA_SMALL_NUMBER)
	{
		Target = DeltaRot.Yaw / DeltaSeconds;
	}

	YawDelta = FMath::FInterpTo(YawDelta, Target, DeltaSeconds, 6.0f);
	RotationLastTick = CharacterRotation;

	// 애니메이션 커브 'FullBody' 값이 0보다 크면 전신 재생 모드로 전환합니다.
	float FullBodyCurveValue = GetCurveValue(TEXT("FullBody"));
	bFullBody = (FullBodyCurveValue > 0.0f);
}
