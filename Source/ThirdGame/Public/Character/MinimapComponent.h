// =========================================================================================
// MinimapComponent.h
//
// [역할 설명]
// 캐릭터 위에 SceneCaptureComponent2D를 붙여 지형을 RT_Minimap에 캡처합니다.
// 캐릭터/VFX는 숨기고 지형/건물만 캡처하여 지도처럼 보이게 합니다.
// MinimapWidget이 이 RenderTarget을 배경으로 깔고, 점(Dot)을 오버레이합니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MinimapComponent.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class THIRDGAME_API UMinimapComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMinimapComponent();

protected:
	virtual void BeginPlay() override;

public:
	// 에디터에서 RT_Minimap 에셋을 연결합니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Minimap")
	UTextureRenderTarget2D* RenderTarget;

	// 미니맵에 표시할 월드 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Minimap")
	float MapRange = 3000.f;

	// 캡처 카메라 높이 (클수록 더 넓게 찍힘)
	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float CaptureHeight = 1500.f;

	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

private:
	UPROPERTY()
	USceneCaptureComponent2D* CaptureComponent;
};
