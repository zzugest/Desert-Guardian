#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapPortal.generated.h"

// ���� ����: �ڽ� �浹 ������Ʈ�� �˱� ����
class UBoxComponent;
class UWidgetComponent;
class UUserWidget;
class AEnemy;

UCLASS()
class THIRDGAME_API AMapPortal : public AActor
{
	GENERATED_BODY()

public:
	AMapPortal();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void InteractWithPortal(class AMyCharacter* PlayerCharacter);

	UFUNCTION()
	void CheckAndApplyPortalState();

	UFUNCTION()
	void OnBossKilled(AEnemy* DeadEnemy);

protected:
	// �÷��̾ ��Ҵ��� �����ϴ� ���� �ڽ�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal Logic")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	UWidgetComponent* InteractPromptWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Settings")
	UDataTable* PortalDataTable; // �츮�� ���� DT_PortalData�� ���� ĭ

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Settings")
	FName PortalRowName; // �� ��Ż�� ������ ���̺��� �� ��° ��(��� ��)���� �ĺ��� �̸�

	//[�߰�] F�� ������ �� ȭ�鿡 ����� '�̵� Ȯ��â UI' ��������Ʈ Ŭ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Settings")
	TSubclassOf<UUserWidget> ConfirmWidgetClass;

	// 수락 버튼 클릭 시 호출 — 저장된 펜딩 데이터로 텔레포트를 실행합니다.
	UFUNCTION()
	void OnConfirmAccepted();

private:
	// 보스 처치 여부 — 서버에서 true로 설정되면 클라이언트에 복제되어 포탈을 활성화합니다.
	UPROPERTY(ReplicatedUsing=OnRep_bBossKilled)
	bool bBossKilled = false;

	UFUNCTION()
	void OnRep_bBossKilled();

	// 확인창 표시 후 수락 시 사용할 이동 데이터
	FName    PendingTargetSubLevelName;
	FName    PendingUnloadSubLevelName;
	FVector  PendingTargetLocation  = FVector::ZeroVector;
	FRotator PendingTargetRotation  = FRotator::ZeroRotator;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// �ڽ��� ������ ����� �� ����� �Լ� (���̳��� ���ε�)
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};