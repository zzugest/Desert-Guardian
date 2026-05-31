#pragma once

#include "CoreMinimal.h"
#include "NPC/BaseNPC.h" 
#include "QuestNPC.generated.h"

class UQuestDialogueWidget;
class UDataTable;
class UWidgetComponent;

UCLASS()
class THIRDGAME_API AQuestNPC : public ABaseNPC
{
    GENERATED_BODY()

protected:
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
    AQuestNPC();

    virtual void InteractWithPlayer(AMyCharacter* PlayerCharacter) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FName> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest UI")
    TSubclassOf<UQuestDialogueWidget> DialogueWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest UI")
    UDataTable* QuestDataTable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    UWidgetComponent* QuestMarkerWidget;

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void UpdateQuestMarker(AMyCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateMarkerUI(int32 MarkerState);

    UFUNCTION()
    void RefreshMarker();

    UFUNCTION()
    void RefreshMarkerByDistance();

private:
    FTimerHandle MarkerDistanceTimerHandle;
};
