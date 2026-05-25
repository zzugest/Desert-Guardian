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

    // �θ��� ��ȣ�ۿ� �Լ��� ����ϴ�
    virtual void InteractWithPlayer(AMyCharacter* PlayerCharacter) override;

    // �� NPC�� �÷��̾�� �� �� �ְų�, �Ϸ���� �� �ִ� ����Ʈ ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FName> AvailableQuests;

    //  ��ȭâ ���� Ŭ������ ���� �ٱ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest UI")
    TSubclassOf<UQuestDialogueWidget> DialogueWidgetClass;

    //  �뺻�� �о�� ���� ǥ(������ ���̺�) ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest UI")
    UDataTable* QuestDataTable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    UWidgetComponent* QuestMarkerWidget;

    // C++���� �÷��̾��� ������ ������ �켱������ ����ϴ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void UpdateQuestMarker(AMyCharacter* PlayerCharacter);

     
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateMarkerUI(int32 MarkerState);


    UFUNCTION()
    void RefreshMarker();

    // 1초마다 거리 체크 후 마커 표시 여부를 결정합니다.
    UFUNCTION()
    void RefreshMarkerByDistance();

private:
    FTimerHandle MarkerDistanceTimerHandle;
};