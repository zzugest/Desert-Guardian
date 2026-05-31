#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapPortal.generated.h"

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

	UFUNCTION(BlueprintCallable, Category = "Portal")
	FName GetTargetSubLevelName() const;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	FName GetCurrentSubLevelName() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal Logic")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	UWidgetComponent* InteractPromptWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Settings")
	UDataTable* PortalDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Settings")
	FName PortalRowName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Settings")
	TSubclassOf<UUserWidget> ConfirmWidgetClass;

	UFUNCTION()
	void OnConfirmAccepted();

private:
	UPROPERTY(ReplicatedUsing=OnRep_bBossKilled)
	bool bBossKilled = false;

	UFUNCTION()
	void OnRep_bBossKilled();

	FName    PendingTargetSubLevelName;
	FName    PendingUnloadSubLevelName;
	FVector  PendingTargetLocation  = FVector::ZeroVector;
	FRotator PendingTargetRotation  = FRotator::ZeroRotator;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
