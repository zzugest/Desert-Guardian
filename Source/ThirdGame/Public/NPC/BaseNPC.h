#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseNPC.generated.h"

class USkeletalMeshComponent;
class UBoxComponent;
class AMyCharacter; 
class UWidgetComponent;

UCLASS()
class THIRDGAME_API ABaseNPC : public AActor
{
    GENERATED_BODY()

public:
    ABaseNPC();

protected:

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
    USkeletalMeshComponent* MeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
    UBoxComponent* InteractionZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|UI")
    UWidgetComponent* InteractPromptWidget;

    UFUNCTION()
    virtual void OnInteractZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnInteractZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    virtual void InteractWithPlayer(AMyCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
    void SetupDialogueState(AMyCharacter* PlayerCharacter, UUserWidget* DialogueWidget);

};
