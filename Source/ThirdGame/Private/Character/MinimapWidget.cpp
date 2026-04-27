// =========================================================================================
// MinimapWidget.cpp
//
// [파일 역할]
// NativePaint를 사용해 매 프레임 미니맵을 직접 그리는 UMG 위젯입니다.
//
// [렌더링 레이어 구조]
// 레이어 1: SceneCapture RenderTarget → 지형 배경 이미지
// 레이어 2: Dot 오버레이
//   - 적(AEnemy)  → 빨간 점
//   - NPC         → 노란 점
//   - 포탈        → 파란 점
//   - 플레이어    → 초록 점  (항상 미니맵 중앙에 고정)
//
// [성능 고려]
// - MinimapSubsystem이 관리하는 단일 목록을 참조해 적·NPC·포탈을 한 번에 순회합니다.
// - MinimapSubsystem 포인터는 NativeConstruct에서 한 번만 캐싱합니다.
// =========================================================================================

#include "MinimapWidget.h"
#include "Rendering/DrawElements.h"
#include "Engine/TextureRenderTarget2D.h"
#include "MinimapSubsystem.h"

// MinimapSubsystem 포인터를 한 번만 캐싱합니다.
void UMinimapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		MinimapSys = GI->GetSubsystem<UMinimapSubsystem>();
	}
}

// MinimapComponent에서 캡처한 RenderTarget을 받아 배경 이미지로 사용하도록 설정합니다.
void UMinimapWidget::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	CapturedRT = InRenderTarget;
}

// 매 프레임 미니맵 배경(RenderTarget)과 적·NPC·플레이어 점(Dot)을 Slate로 직접 그립니다.
int32 UMinimapWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	int32 Layer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return Layer;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return Layer;

	FVector PlayerPos = Pawn->GetActorLocation();
	FVector2D ViewportSize  = AllottedGeometry.GetLocalSize();
	FVector2D MinimapOrigin = FVector2D(ViewportSize.X - MinimapMargin.X, MinimapMargin.Y);

	// ── 레이어 1: 지형 배경 (SceneCapture RenderTarget) ──
	if (CapturedRT)
	{
		FSlateBrush RTBrush;
		RTBrush.SetResourceObject(CapturedRT);
		RTBrush.DrawAs   = ESlateBrushDrawType::Image;
		RTBrush.ImageType = ESlateBrushImageType::FullColor;
		RTBrush.ImageSize = MinimapSize;

		FSlateDrawElement::MakeBox(
			OutDrawElements, ++Layer,
			AllottedGeometry.ToPaintGeometry(MinimapSize, FSlateLayoutTransform(MinimapOrigin)),
			&RTBrush, ESlateDrawEffect::None,
			TerrainTint);
	}
	else
	{
		// RenderTarget이 없으면 반투명 검정 배경으로 대체합니다.
		FSlateColorBrush FallbackBrush(FLinearColor::White);
		FSlateDrawElement::MakeBox(
			OutDrawElements, ++Layer,
			AllottedGeometry.ToPaintGeometry(MinimapSize, FSlateLayoutTransform(MinimapOrigin)),
			&FallbackBrush, ESlateDrawEffect::None,
			FLinearColor(0.f, 0.f, 0.f, 0.75f));
	}

	// ── 레이어 2: Dot 오버레이 ──

	// MinimapSubsystem의 단일 목록을 순회해 적·NPC·포탈을 한 번에 그립니다.
	if (MinimapSys)
	{
		for (const FMinimapMarker& Marker : MinimapSys->GetMarkers())
		{
			if (!IsValid(Marker.Actor)) continue;

			FLinearColor Color;
			switch (Marker.Type)
			{
				case EMinimapMarkerType::Enemy:  Color = FLinearColor::Red;    break;
				case EMinimapMarkerType::NPC:    Color = FLinearColor::Yellow; break;
				case EMinimapMarkerType::Portal: Color = FLinearColor::Blue;   break;
				default:                         Color = FLinearColor::White;  break;
			}

			DrawDot(OutDrawElements, AllottedGeometry, Layer, MinimapOrigin,
				WorldToMinimap(Marker.Actor->GetActorLocation(), PlayerPos), 7.f, Color);
		}
	}

	// 플레이어 — 초록 점 (항상 미니맵 중앙, 최상위 레이어)
	DrawDot(OutDrawElements, AllottedGeometry, Layer, MinimapOrigin,
		MinimapSize * 0.5f, 10.f, FLinearColor::Green);

	return Layer;
}

// 월드 좌표를 미니맵 로컬 UV 좌표로 변환합니다.
// SceneCapture Pitch=-90° 기준: 텍스처 +U = 월드 +Y, 텍스처 +V = 월드 -X
FVector2D UMinimapWidget::WorldToMinimap(FVector ActorPos, FVector PlayerPos) const
{
	float DX = ActorPos.X - PlayerPos.X;
	float DY = ActorPos.Y - PlayerPos.Y;

	float U = ( DY / (MapRange * 2.f) + 0.5f) * MinimapSize.X;
	float V = (-DX / (MapRange * 2.f) + 0.5f) * MinimapSize.Y;

	return FVector2D(U, V);
}

// 미니맵 범위 안에 있는 액터를 지정한 색상의 점으로 그립니다. 범위 밖이면 그리지 않습니다.
void UMinimapWidget::DrawDot(
	FSlateWindowElementList& OutDrawElements,
	const FGeometry& AllottedGeometry,
	int32 LayerId,
	FVector2D MinimapOrigin,
	FVector2D LocalPos,
	float DotSize,
	FLinearColor Color) const
{
	if (LocalPos.X < 0.f || LocalPos.X > MinimapSize.X ||
		LocalPos.Y < 0.f || LocalPos.Y > MinimapSize.Y)
	{
		return;
	}

	FSlateColorBrush Brush(FLinearColor::White);
	FVector2D DrawPos = MinimapOrigin + LocalPos - FVector2D(DotSize * 0.5f, DotSize * 0.5f);

	FSlateDrawElement::MakeBox(
		OutDrawElements, LayerId + 1,
		AllottedGeometry.ToPaintGeometry(FVector2D(DotSize, DotSize), FSlateLayoutTransform(DrawPos)),
		&Brush, ESlateDrawEffect::None, Color);
}
