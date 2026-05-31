#include "MyGameModeBase.h"
#include "NiagaraFunctionLibrary.h" // Niagara 시스템 스폰을 위해 필요합니다!

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// 배열에 등록된 모든 VFX 이펙트를 순회하며 사전 로드(Pre-warm) 합니다.
	for (UNiagaraSystem* VFX : PreloadVFXList)
	{
		if (VFX)
		{
			// 플레이어 눈에 보이지 않는 지면 아래 숨겨진 위치에서 스폰합니다.
			FVector HiddenLocation = FVector(0.0f, 0.0f, -10000.0f);

			// 숨겨진 위치에서 이펙트를 1회 재생해 엔진 오브젝트 풀(Pool)에 미리 등록합니다.
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				VFX,
				HiddenLocation,
				FRotator::ZeroRotator,
				FVector(1.0f),
				true,
				true,
				ENCPoolMethod::AutoRelease // [참고] 재생 완료 후 자동으로 풀에 반납됩니다!
			);
		}
	}

	// [확인 로그] 필요 시 아래 로그를 활성화합니다.
	//UE_LOG(LogTemp, Warning, TEXT("=== VFX %d개 사전 로드 및 풀 등록 완료! ==="), PreloadVFXList.Num());
}