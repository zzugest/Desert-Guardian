// =========================================================================================
// MinimapComponent.cpp
//
// [파일 역할]
// 캐릭터 머리 위에 SceneCaptureComponent2D를 동적으로 생성해 미니맵 렌더 타겟을 채우는 컴포넌트입니다.
// 캡처 카메라는 직교 투영(Orthographic) + 항상 정북 방향 고정으로 설정하여
// 지도처럼 보이도록 하며, 캐릭터·VFX·그림자 등은 ShowFlags로 숨깁니다.
// =========================================================================================

#include "MinimapComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

// Tick이 불필요하므로 비활성화합니다.
UMinimapComponent::UMinimapComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// 게임 시작 시 SceneCaptureComponent2D를 생성해 캐릭터 위에 부착하고 미니맵 촬영 설정을 적용합니다.
void UMinimapComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	CaptureComponent = NewObject<USceneCaptureComponent2D>(Owner, TEXT("MinimapCapture"));
	if (!CaptureComponent) return;

	CaptureComponent->RegisterComponent();
	CaptureComponent->AttachToComponent(
		Owner->GetRootComponent(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// 위치는 캐릭터를 따라가되, 회전은 항상 월드 기준 정북을 향하도록 고정합니다.
	// bAbsoluteRotation = true → 부모(캐릭터)의 Yaw 회전을 무시
	CaptureComponent->SetRelativeLocation(FVector(0.f, 0.f, CaptureHeight));
	CaptureComponent->SetUsingAbsoluteRotation(true);
	CaptureComponent->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));

	// 직교 투영: 원근감 없이 평면으로 캡처해 지도 느낌을 줍니다.
	CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	CaptureComponent->OrthoWidth     = MapRange * 2.f;

	// FinalColorLDR: 알파 채널이 1로 고정되어 투명 문제가 없습니다.
	CaptureComponent->CaptureSource     = ESceneCaptureSource::SCS_FinalColorLDR;
	CaptureComponent->bCaptureEveryFrame = true;
	CaptureComponent->bCaptureOnMovement = false;

	// 지형·건물만 보이고 캐릭터, VFX, 그림자 등은 숨겨 지도처럼 표시합니다.
	CaptureComponent->ShowFlags.SetSkeletalMeshes(false);  // 캐릭터/NPC 숨김
	CaptureComponent->ShowFlags.SetParticles(false);        // VFX 숨김
	CaptureComponent->ShowFlags.SetAtmosphere(false);       // 하늘 대기 숨김
	CaptureComponent->ShowFlags.SetFog(false);              // 안개 숨김
	CaptureComponent->ShowFlags.SetDecals(false);           // 데칼 숨김
	CaptureComponent->ShowFlags.SetDynamicShadows(false);   // 그림자 숨김

	if (RenderTarget)
	{
		CaptureComponent->TextureTarget = RenderTarget;
	}
}
