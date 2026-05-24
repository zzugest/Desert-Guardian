#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

//  ���� ����
class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UProjectileMovementComponent;

UCLASS()
class THIRDGAME_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

protected:
	virtual void BeginPlay() override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

public:
	// �浹ü (�ε����� ����)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComp;

	// ���̾ư��� ����Ʈ (���̾, ���� ȸ���� ��)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* NiagaraComp;

	// �߻�ü �̵� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// �⺻ ������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 50.0f;

	// ���ο� ����ü ��ȯ �ÿ� ��ģ ���̾ư�� ����Ʈ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UNiagaraSystem* HitEffect = nullptr;

	// ���� ����� �� ����� �Լ�
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 서버 → 모든 클라이언트: 투사체 피격 이펙트를 모든 화면에서 재생합니다.
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastOnHit(FVector Location, FRotator Rotation);

	// 스폰 시점에 시전자를 볼 수 있었던 플레이어 컨트롤러 목록입니다.
	TArray<APlayerController*> RelevantControllers;
};