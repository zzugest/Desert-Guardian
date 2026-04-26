#include "ObjectPoolSubsystem.h"
#include "GameFramework/Actor.h"

AActor* UObjectPoolSubsystem::GetPooledActor(TSubclassOf<AActor> ClassToSpawn)
{
    // 유효하지 않은 클래스 요청 시 예외 처리
    if (!ClassToSpawn) return nullptr;

    // 대상 클래스의 풀(Pool) 존재 여부 및 가용 액터 확인
    if (ObjectPool.Contains(ClassToSpawn) && ObjectPool[ClassToSpawn].PooledActors.Num() > 0)
    {
        // 풀 대기열에서 액터 추출
        AActor* PooledActor = ObjectPool[ClassToSpawn].PooledActors.Pop();

        if (PooledActor)
        {
            // 추출한 액터 반환
            return PooledActor;
        }
    }

    // 가용 액터가 없을 경우 nullptr 반환 (호출부에서 신규 생성 유도)
    return nullptr;
}

void UObjectPoolSubsystem::ReturnActorToPool(AActor* ActorToReturn)
{
    // 유효하지 않은 액터 반환 요청 시 예외 처리
    if (!ActorToReturn) return;

    // 1. 액터 비활성화 처리 (렌더링 및 물리 충돌 해제)
    ActorToReturn->SetActorHiddenInGame(true);
    ActorToReturn->SetActorEnableCollision(false);

    // 2. 반환 대상 액터의 클래스 정보 획득
    UClass* ActorClass = ActorToReturn->GetClass();

    // 3. 해당 클래스를 키(Key)로 하여 중앙 풀(TMap) 대기열에 액터 추가
    ObjectPool.FindOrAdd(ActorClass).PooledActors.Add(ActorToReturn);
}

void UObjectPoolSubsystem::PrintPoolStatus()
{
    //if (!GEngine) return;

    //// 맨 위에 제목 줄 하나 띄우기 (노란색, 5초 동안)
    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("=== Current Object Pool Status ==="));

    //// TMap인 ObjectPool을 순회하면서 재고 파악하기
    //for (const auto& PoolPair : ObjectPool)
    //{
    //    UClass* ActorClass = PoolPair.Key;
    //    int32 SleepingCount = PoolPair.Value.PooledActors.Num(); // 수면실에 있는 액터 개수

    //    if (ActorClass)
    //    {
    //        // "Class: OOO | Pooled Actors: X" 형태로 글자 조합
    //        FString DebugMsg = FString::Printf(TEXT("Class: %s | Pooled Actors: %d"), *ActorClass->GetName(), SleepingCount);

    //        // 화면에 출력 (하늘색, 5초 동안)
    //        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, DebugMsg);
    //    }
    //}
}