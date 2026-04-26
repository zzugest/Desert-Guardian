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

private:
	bool bBossKilled = false;
	

	virtual void BeginPlay() override;

	// �ڽ��� ������ ����� �� ����� �Լ� (���̳��� ���ε�)
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};