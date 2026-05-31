
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "Perception/AIPerceptionTypes.h"
#include "NPC/Quest/QuestData.h"
#include "GameplayTagContainer.h"
#include "Enemy.generated.h"

class UWidgetComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDiedSignature, class AEnemy*, DiedMonster);

USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	FName EnemyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	FText EnemyDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float MoveSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float Phase2MoveSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	TSubclassOf<UAnimInstance> AnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float DropExp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float AttackCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBehaviorTree* BehaviorTree = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float BaseAttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<class UAnimMontage*> BaseAttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	FVector MeshScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	float CapsuleRadius = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	float CapsuleHalfHeight = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	float HPBarHeight = 150.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<class AEnemy> EnemyClass;
};

UCLASS()
class THIRDGAME_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_bIsDead();

	UFUNCTION()
	void OnRep_EnemySetup();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHP = 100.0f;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentHP, VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float CurrentHP;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* HPBarWidget;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(ReplicatedUsing=OnRep_EnemySetup, EditAnywhere, BlueprintReadWrite, Category = "Enemy Setup")
	UDataTable* EnemyDataTable;

	UPROPERTY(ReplicatedUsing=OnRep_EnemySetup, EditAnywhere, BlueprintReadWrite, Category = "Enemy Setup")
	FName EnemyRowName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Setup")
	float AttackPower;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Setup")
	float AttackRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Setup")
	float AttackCooldown;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void ExecuteAttackHit();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<class UAnimMontage*> BaseAttackMontages;

	virtual void BaseAttack();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAttackMontage(UAnimMontage* Montage);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateQuestObjective(EQuestTaskType TaskType, FName TargetRowName);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bCanAttack = true;

	FTimerHandle AttackTimerHandle;

	FTimerHandle DeathTimerHandle;

	virtual void ResetAttack();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAIPerceptionComponent* AIPerceptionComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAISenseConfig_Sight* SightConfig = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasDamaged = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseAttackDamage = 0.0f;

	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY()
	class UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	FVector HomeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolRadius = 1500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UNavigationInvokerComponent* NavInvoker;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMonsterDiedSignature OnMonsterDied;

	UFUNCTION(BlueprintCallable, Category = "State")
	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = "State")
	virtual void Revive();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* DeathMontage;

	UPROPERTY(ReplicatedUsing=OnRep_bIsDead)
	bool bIsDead = false;

	UPROPERTY()
	class AMonsterSpawner* MySpawner;

	UPROPERTY(EditAnywhere, Category = "AI")
	float LeashRadius = 1500.0f;

	bool bIsReturning = false;

	FTimerHandle LeashTimerHandle;

	virtual void CheckLeash();

	UPROPERTY()
	float OriginalMoveSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
	UStaticMeshComponent* TargetMarkerMesh;

	void SetTargetMarkerVisibility(bool bShow);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State|Targeting")
	bool bIsTargetable = true;

	UPROPERTY()
	APlayerCameraManager* CachedCameraManager;

	FText MyDisplayName;

	UPROPERTY()
	FGameplayTag CurrentHitType;

};
