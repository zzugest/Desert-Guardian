// =========================================================================================
// MinimapWidget.h
//
// [역할 설명]
// NativePaint로 두 레이어를 그립니다.
//  레이어 1 (배경): SceneCaptureComponent2D가 캡처한 RT를 지도처럼 표시
//  레이어 2 (오버레이): 플레이어(초록), 적(빨강), NPC(노랑)를 점으로 표시
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MinimapWidget.generated.h"

class UTextureRenderTarget2D;
class AMyCharacter;

UCLASS()
class THIRDGAME_API UMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// MinimapComponent로부터 RenderTarget을 받아 배경으로 사용합니다.
	void SetRenderTarget(UTextureRenderTarget2D* InRenderTarget);

	// 미니맵에 표시할 월드 반경 (MinimapComponent의 MapRange와 동일하게 설정)
	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float MapRange = 3000.f;

	// 미니맵 표시 크기 (픽셀)
	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	FVector2D MinimapSize = FVector2D(200.f, 200.f);

	// 화면 우상단 여백 (X: 오른쪽 끝에서 안쪽, Y: 위에서 아래)
	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	FVector2D MinimapMargin = FVector2D(220.f, 20.f);

	// 지형 배경 색조 (지도처럼 보이게 하는 색상 곱하기, 기본값: 황토빛)
	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	FLinearColor TerrainTint = FLinearColor(0.6f, 0.55f, 0.45f, 1.f);

protected:
	virtual void NativeConstruct() override;

	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	// SceneCapture 결과 텍스처 (배경으로 사용)
	UPROPERTY()
	UTextureRenderTarget2D* CapturedRT = nullptr;

	// MinimapSubsystem 캐시 (NativePaint에서 매 프레임 GetSubsystem 호출 방지)
	UPROPERTY()
	class UMinimapSubsystem* MinimapSys = nullptr;

	// 월드 좌표 → 미니맵 로컬 픽셀 좌표 변환
	FVector2D WorldToMinimap(FVector ActorPos, FVector PlayerPos) const;

	// 미니맵 범위 안에 있으면 점을 그림
	void DrawDot(
		FSlateWindowElementList& OutDrawElements,
		const FGeometry& AllottedGeometry,
		int32 LayerId,
		FVector2D MinimapOrigin,
		FVector2D LocalPos,
		float DotSize,
		FLinearColor Color) const;
};
