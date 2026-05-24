// =========================================================================================
// Enemy.h
//
// [���� ���]
// ���� �� ��� ��(����) ���Ͱ� �������� ��ӹ޴� ���̽� Ŭ���� ����Դϴ�.
// ���������̺� ��� ���� �ʱ�ȭ���� ���, ��Ȱ, ���� ������Ʈ ����, AI �þ� ����, �ڵ� ������ ��ȯ(Leash) ������ �ƿ츣�� ������ ��� �ֽ��ϴ�.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "Perception/AIPerceptionTypes.h"
#include "Enemy.generated.h"

class UWidgetComponent;
class UStaticMeshComponent;

// ���� ��� �� ����Ʈ/������ ���� ���� �˸��� ���� ��������Ʈ�Դϴ�.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDiedSignature, class AEnemy*, DiedMonster);

// ���������̺��� ���ǵǾ� �ִ� ������ �⺻ ����(����/����/AI) ����ü�Դϴ�.
USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// ���� �ĺ��� �̸��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	FName EnemyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	FText EnemyDisplayName;

	// ������ �ִ� ü���Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float MaxHP;

	// ������ ���� �� ������ �� ���ݷ� ��ġ�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float AttackPower;

	// �̵� �� ���� �ӵ��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float MoveSpeed = 500.0f;

	// 2페이즈 진입 시 적용할 이동 속도입니다. 0이면 속도 변화 없음 (보스 전용).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float Phase2MoveSpeed = 0.0f;


	// ���� �ִϸ��̼��� ������ �ִϸ��̼� ��������Ʈ Ŭ�����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	TSubclassOf<UAnimInstance> AnimClass;

	// óġ �� �÷��̾�� ���޵� ȹ�� ����ġ�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float DropExp;

	// ���� ������ �� �ִ� �ִ� ��Ÿ��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float AttackRange;

	// ���� ������ �����ϴ� ���� ���� �غ������ ��� �ð�(��)�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float AttackCooldown;

	// ���� AI ������ ������ �����̺�� Ʈ�� �����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBehaviorTree* BehaviorTree = nullptr;

	// �⺻ ���� ��Ÿ�ֿ� �Բ� ����� �߰� ���� ���ݷ��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	float BaseAttackDamage = 10.0f;

	// �Ϲ� ���� �� ����� �ִϸ��̼� ��Ÿ�� ����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<class UAnimMontage*> BaseAttackMontages;

	// ���� ������ ���� ���� �����ϰ��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	FVector MeshScale = FVector(1.0f, 1.0f, 1.0f);

	// �浹 ���� ĸ���� ���� �ݰ��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	float CapsuleRadius = 40.0f;

	// �浹 ���� ĸ���� ���� ����(����)�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	float CapsuleHalfHeight = 90.0f;

	// ĳ���� ��ܿ� ����� ü�¹��� Z(������) ������ ���ذ��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	float HPBarHeight = 150.0f;
	

	// ���� ���� ȯ�濡 ������ų ������ ���� ��������Ʈ Ŭ�����Դϴ�.
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

	// ���� ������ ���� �������� �Ǵ� ������ ��ü ������ �Ѱ�ġ�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHP = 100.0f;

	// �������� ���� �����Ǵ� ���� ������ ��ġ�Դϴ�.
	UPROPERTY(ReplicatedUsing=OnRep_CurrentHP, VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float CurrentHP;

	// ���� �浹 ���� �����κ��� �ǰ� �̺�Ʈ�� �����Ͽ� ��ü���� ������/��� ó���� �����մϴ�.
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// ����� �Ӹ� ���� ���� ���� ������ ������ ����� UI ���� ��ü�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* HPBarWidget;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ĳ������ �浹ü(ĸ��)�� Ÿ ���Ϳ� ���������� �´���� �� ����Ǵ� �̺�Ʈ�Դϴ�.
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// ���� ������ �Ҵ�� ���� ���� ���������̺��Դϴ�.
	UPROPERTY(ReplicatedUsing=OnRep_EnemySetup, EditAnywhere, BlueprintReadWrite, Category = "Enemy Setup")
	UDataTable* EnemyDataTable;

	// ���������̺� ������ ���� ������ �����͸� Ư���ϱ� ���� ��(Row) �̸� ���Դϴ�.
	UPROPERTY(ReplicatedUsing=OnRep_EnemySetup, EditAnywhere, BlueprintReadWrite, Category = "Enemy Setup")
	FName EnemyRowName;

	// ������ ���Ŀ� Ȱ��Ǵ� �⺻ ���ȿ� ��ü�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Setup")
	float AttackPower;

	// ����� ���� ������ ������ �� �ִ� ��ȿ �Ÿ��Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Setup")
	float AttackRange;

	// �⺻ ���� �� ������� �������� �������� ��� �ð�(��)�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Setup")
	float AttackCooldown;

	// �ִϸ��̼� ��Ƽ���̿� ���� ������ �÷��̾��� ü���� ��� ������ �߻���ŵ�ϴ�.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void ExecuteAttackHit();

	// ���� �Ͽ� ���� ���Ƿ� ����� ��Ÿ�� �ĺ��� �迭�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<class UAnimMontage*> BaseAttackMontages;

	// ������ ���� ����(���� ��)�� �����Ǿ��� �� �⺻ ��Ÿ�ָ� �����Ű�� ��ٿ��� �����մϴ�.
	virtual void BaseAttack();

	// 서버에서 선택한 공격 몽타주를 모든 클라이언트에 동기화해 재생합니다.
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAttackMontage(UAnimMontage* Montage);

	// ���� ���Ͱ� ��Ÿ�Ӱ� �����ϰ� ������ �õ��� �� �ִ� �������� �Ǵ��ϴ� �÷����Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bCanAttack = true;

	// ������ �ð� ���Ŀ� ���� �÷��׸� ������ų Ÿ�̸� �����Դϴ�.
	FTimerHandle AttackTimerHandle;

	// 사망 애니메이션 종료 후 Die()를 호출하는 타이머 핸들.
	// 멤버로 유지해야 Revive() 시 ClearTimer로 취소할 수 있습니다.
	FTimerHandle DeathTimerHandle;

	// ��Ÿ�� Ÿ�̸ӿ� ���� ȣ��Ǿ� �ٽ� ���� ������ ���·� �ǵ����ϴ�.
	virtual void ResetAttack();

	// AI�� "�þ�(Sight)"�� �����ϸ� �߰� �̺�Ʈ�� ������ ������Ʈ ��ü�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAIPerceptionComponent* AIPerceptionComp = nullptr;

	// AI�� ����(����, �ν� �ݰ�) ���� ���� ������ �����մϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAISenseConfig_Sight* SightConfig = nullptr;

	// �ٴ� ��Ʈ�� ���� �ߺ� �������� �混�ϰ� ���� ���� ���� ���°��Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasDamaged = false;

	// ���� �� ����� ���� �������Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseAttackDamage = 0.0f;

	// AI �ۼ��ǿ� ���� ����(�÷��̾� ��)�� �þ߿� �����ų� ������ �� �ߵ��ϴ� �ݹ� �̺�Ʈ�Դϴ�.
	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

	// ������ �� ���� � ����� ���� ������ ���� ������Ʈ�Դϴ�.
	UPROPERTY()
	class UStaticMeshComponent* WeaponMesh;

	// �����ʷκ��� ������ �� ��ϵǾ��� ó������ �Ƚ�ó(��׷� Ǯ��) ��ġ�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	FVector HomeLocation;

	// ��(Home) �ֺ��� �ɵ� ������ ������ ���� �ݰ��� ���� ��ġ�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolRadius = 1500.0f;

	// �� ��ü�� �ƴ� ���� �ݰ� �Ϻο����� ������̼� Ÿ��(�� ã�� ����)�� ������ �� �ֵ��� ���� ������Ʈ�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UNavigationInvokerComponent* NavInvoker;

	// ���� ���� ��� �� �ܺ� ���(�������� ��Ȱ ���� ��)�� �̸� ĳġ�� �� �ֵ��� ���ִ� ��������Ʈ�Դϴ�.
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMonsterDiedSignature OnMonsterDied;

	// ���� ü���� 0�� �Ǿ��� �� ���¸� ����(Dead)�� ��ȯ�ϰ� �ݸ����� �����Ͽ� ���͸� ��üȭ�մϴ�.
	UFUNCTION(BlueprintCallable, Category = "State")
	virtual void Die();

	// �����ʿ� ���� Ȱ��ȭ�Ǿ� ������� ��ã�� �� ü�°� AI�� ������� �۵���ŵ�ϴ�.
	UFUNCTION(BlueprintCallable, Category = "State")
	virtual void Revive();

	// ��� �� ����! �ϰ� ������ �� ����Ǵ� �׼� �ִϸ��̼��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* DeathMontage;

	// �̹� ����Ͽ� ��üȭ�� ��ü�� Ÿ���� ���� �� ���׼� �ߺ� ó���� �Ǵ� ���� ���� �����Դϴ�.
	UPROPERTY(ReplicatedUsing=OnRep_bIsDead)
	bool bIsDead = false;

	// Ǯ��(Pooling) �������� �ڽ��� ȣ���ϰ� ȸ���ϴ� ���� ���� �������� ������ �������Դϴ�.
	UPROPERTY()
	class AMonsterSpawner* MySpawner;

	// ���� ����� �����ϴ��� ����(�Ƚ�ó) ��ġ�κ��� �ִ�� ����� �� �ִ� ��� �Ÿ�(����) ��ġ�Դϴ�.
	UPROPERTY(EditAnywhere, Category = "AI")
	float LeashRadius = 1500.0f;

	// �ָ� �̵��Ǿ� ��(Home) ��ġ�� ��ȯ�� �õ��ϰ� �ִ��� �ľ��ϱ� ���� �÷����Դϴ�. 
	bool bIsReturning = false;

	// ��ȯ ���� ��Ż�� �ֱ������� �˻��ϴ� Ÿ�̸� �����Դϴ�.
	FTimerHandle LeashTimerHandle;

	// �ֱ������� ���� �Ÿ��� �˻��� �Ѱ踦 �Ѿ��ٸ� ��ȯ�� �����մϴ�.
	virtual void CheckLeash();

	// ĳ���Ͱ� ��ȯ ��� ��� �޸����� ����� �ӵ��� ���� �ڽ��� �� �ӵ��� �����ϱ� ���� ����� �������Դϴ�.
	UPROPERTY()
	float OriginalMoveSpeed;

	// �÷��̾ �ش� ���Ϳ� Ÿ����(Lock-On)�� �������� �� �Ӹ��� ���߿� ����� �ǵ�� ��Ŀ�� �޽��Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
	UStaticMeshComponent* TargetMarkerMesh;

	// �ý����� Ÿ�� ���� �˸��� �޾� �ð� ��Ŀ ������Ʈ�� On/Off ȭ ��ŵ�ϴ�.
	void SetTargetMarkerVisibility(bool bShow);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State|Targeting")
	bool bIsTargetable = true;

	// [비활성화] 타겟팅 → Overlap Sphere / 미니맵 → MinimapSubsystem으로 대체되어 더 이상 사용하지 않습니다.
	// static TArray<TWeakObjectPtr<AEnemy>> AllActiveEnemies;

	// BeginPlay에서 한 번만 조회해 캐싱 — Tick마다 GetPlayerCameraManager() 호출 방지
	UPROPERTY()
	APlayerCameraManager* CachedCameraManager;

	FText MyDisplayName;
};