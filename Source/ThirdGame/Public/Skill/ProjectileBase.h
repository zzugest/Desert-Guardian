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
};