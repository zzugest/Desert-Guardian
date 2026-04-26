#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

//  1. TArray를 감싸줄 포장지(구조체)를 하나 만듭니다.
USTRUCT()
struct FPoolArray
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<AActor*> PooledActors;
};

UCLASS()
class THIRDGAME_API UObjectPoolSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Pool")
    AActor* GetPooledActor(TSubclassOf<AActor> ClassToSpawn);

    UFUNCTION(BlueprintCallable, Category = "Pool")
    void ReturnActorToPool(AActor* ActorToReturn);


    UFUNCTION(BlueprintCallable, Category = "ObjectPool|Debug")
    void PrintPoolStatus();

private:
    //  2. TArray 대신 방금 만든 포장지(FPoolArray)를 값으로 사용합니다!
    UPROPERTY()
    TMap<UClass*, FPoolArray> ObjectPool;
};