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

    // 모든 NPC가 공통으로 가질 몸통(Mesh)과 범위(Box)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
    USkeletalMeshComponent* MeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
    UBoxComponent* InteractionZone;

    // "F 키로 대화" 같은 문구를 띄워줄 NPC 전용 위젯 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|UI")
    UWidgetComponent* InteractPromptWidget;

    // 플레이어가 박스에 들어왔을 때 실행될 함수
    UFUNCTION()
    virtual void OnInteractZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // 플레이어가 박스에서 나갔을 때 실행될 함수
    UFUNCTION()
    virtual void OnInteractZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    // 자식 클래스들이 각자 입맛에 맞게 덮어쓸 상호작용 가상 함수
    virtual void InteractWithPlayer(AMyCharacter* PlayerCharacter);

    // 자식 NPC들이 대화창을 띄울 때 플레이어를 통제하기 위해 부를 공통 함수
    UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
    void SetupDialogueState(AMyCharacter* PlayerCharacter, UUserWidget* DialogueWidget);


    
};