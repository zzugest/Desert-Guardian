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

    // ��� NPC�� �������� ���� ����(Mesh)�� ����(Box)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
    USkeletalMeshComponent* MeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
    UBoxComponent* InteractionZone;

    // "F Ű�� ��ȭ" ���� ������ ����� NPC ���� ���� ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|UI")
    UWidgetComponent* InteractPromptWidget;

    // �÷��̾ �ڽ��� ������ �� ����� �Լ�
    UFUNCTION()
    virtual void OnInteractZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // �÷��̾ �ڽ����� ������ �� ����� �Լ�
    UFUNCTION()
    virtual void OnInteractZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    // �ڽ� Ŭ�������� ���� �Ը��� �°� ��� ��ȣ�ۿ� ���� �Լ�
    virtual void InteractWithPlayer(AMyCharacter* PlayerCharacter);

    // �ڽ� NPC���� ��ȭâ�� ��� �� �÷��̾ �����ϱ� ���� �θ� ���� �Լ�
    UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
    void SetupDialogueState(AMyCharacter* PlayerCharacter, UUserWidget* DialogueWidget);


    
};