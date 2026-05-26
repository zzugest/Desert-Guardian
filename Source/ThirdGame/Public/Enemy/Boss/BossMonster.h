#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h"
#include "Perception/AIPerceptionTypes.h"
#include "Components/DecalComponent.h"
#include "BossMonster.generated.h"

class UBossHPWidget;

UCLASS()
class THIRDGAME_API ABossMonster : public AEnemy
{
	GENERATED_BODY()

public:
	ABossMonster();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ── Rep 콜백 ──────────────────────────────────────────────────────────
	UFUNCTION()
	void OnRep_bIsRagePhase();

	UFUNCTION()
	void OnRep_bIsJumpAttacking();

	// ── Multicast RPC ──────────────────────────────────────────────────────
	// 보스 최초 감지 시 모든 클라이언트에 HP 바를 표시합니다.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowBossUI();

	// 2페이즈 진입 시 변신 몽타주·HP 바를 모든 클라이언트에 동기화합니다.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EnterRagePhase();

	// 보스 HP가 변경될 때마다 모든 클라이언트의 HP 바를 갱신합니다.
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_UpdateBossHP(float NewHP, float NewMaxHP);

	// 변신 애니메이션 완료 후 이름과 HP를 모든 클라이언트에 갱신합니다.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnRageComplete(float NewHP, float NewMaxHP);

	// 점프 공격 경고 데칼을 모든 클라이언트에 스폰합니다.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnWarningDecal(FVector DecalLocation);

	// 착지 후 경고 데칼을 모든 클라이언트에서 제거합니다.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RemoveWarningDecal();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void CheckLeash() override;

	virtual void Revive() override;

	virtual void Die() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI")
	TSubclassOf<UBossHPWidget> BossHPWidgetClass;

	UPROPERTY()
	UBossHPWidget* BossUI;

	// 보스가 플레이어를 이미 감지했는지 (HP바 중복 표시 방지)
	bool bIsEngaged = false;

	// Perception 콜백: 플레이어 최초 감지 시 HP바를 띄움
	UFUNCTION()
	void OnBossTargetDetected(AActor* Actor, FAIStimulus Stimulus);

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	float Phase2_HP_Ratio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	UAnimMontage* TransformationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sound")
	USoundBase* BossBGM;

	UPROPERTY(ReplicatedUsing=OnRep_bIsRagePhase, BlueprintReadWrite, Category = "Boss|State")
	bool bIsRagePhase = false;

	void CheckPhaseTransition();

	void EnterRagePhase();

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	void EndRageTransformation(UAnimMontage* Montage, bool bInterrupted);

	virtual void BaseAttack() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Combat")
	TArray<UAnimMontage*> RageAttackMontages;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Combat")
	UAnimMontage* JumpAttackMontage;

	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void ExecuteJumpAttack();

	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void LaunchToAir();

	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	virtual void Landed(const FHitResult& Hit) override;


	// DataTable에서 읽어온 2페이즈 이동 속도 캐시값입니다.
	UPROPERTY()
	float Phase2MoveSpeed = 0.0f;

	// 점프 공격 경고 범위를 표시할 데칼 머티리얼 (에디터에서 할당)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Combat")
	UMaterialInterface* WarningDecalMaterial = nullptr;

	// 경고 데칼 반지름 — AN_GroundSlam의 DamageRadius와 맞춰야 함
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Combat")
	float WarningDecalRadius = 150.0f;

	// 현재 스폰된 경고 데칼 포인터
	UPROPERTY()
	UDecalComponent* WarningDecal = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI")
	FString Phase1Name = TEXT("보스 기본 이름");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI")
	FString Phase2Name = TEXT("분노한 보스 이름");


protected:
	UPROPERTY()
	ACharacter* TargetPlayer;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Boss|State")
	bool bIsInvincible = false;

	UPROPERTY(ReplicatedUsing=OnRep_bIsJumpAttacking, VisibleAnywhere, BlueprintReadWrite, Category = "Boss|State")
	bool bIsJumpAttacking = false;
};
