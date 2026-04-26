#include "MyGameModeBase.h"
#include "NiagaraFunctionLibrary.h" // ���̾ư��� ������ ���� �� �ʿ��մϴ�!

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// �迭�� ��ϵ� ��� ����Ʈ�� ��ȸ�ϸ� ����(Pre-warm) �մϴ�.
	for (UNiagaraSystem* VFX : PreloadVFXList)
	{
		if (VFX)
		{
			// �÷��̾� ���� ���� ������ �ʴ� �� �� ������ �� ��ǥ
			FVector HiddenLocation = FVector(0.0f, 0.0f, -10000.0f);

			// ���̴� �������� ������ 1ȸ ��Ű��, ��� ����(Pool)�� �ݳ��մϴ�.
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				VFX,
				HiddenLocation,
				FRotator::ZeroRotator,
				FVector(1.0f),
				true,
				true,
				ENCPoolMethod::AutoRelease // [�ٽ�] �ı����� ���� Ǯ���ض�!
			);
		}
	}

	// [Ȯ�ο� �α�] ���߿� ����ŵ� �˴ϴ�.
	//UE_LOG(LogTemp, Warning, TEXT("=== VFX %d�� ���� �� Ǯ�� �Ϸ�! ==="), PreloadVFXList.Num());
}