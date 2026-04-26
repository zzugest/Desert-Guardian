#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossRoomTrigger.generated.h"

class UBoxComponent;
class ULevelSequence;
class AMyCharacter;

UCLASS()
class THIRDGAME_API ABossRoomTrigger : public AActor
{
	GENERATED_BODY()

public:
	ABossRoomTrigger();

protected:
	// �÷��̾ ���� ������ �ڽ�
	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	UBoxComponent* CollisionBox;

	// �����Ϳ��� ����� �츮�� ���� �ƽ�(���� ������) ����
	UPROPERTY(EditAnywhere, Category = "Cinematic")
	ULevelSequence* BossIntroSequence;

	// �ƽ��� �� �� ����Ǵ� ���� ���� ���� ����
	bool bHasPlayed = false;

	UPROPERTY()
	AMyCharacter* CachedPlayer = nullptr;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCutsceneFinished();
};