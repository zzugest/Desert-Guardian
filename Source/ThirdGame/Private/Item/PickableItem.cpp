// =========================================================================================
// PickableItem.cpp
//
// [파일 역할]
// 월드에 배치된 줍기 가능한 아이템 액터입니다.
// BeginPlay에서 DataTable 행을 읽어 RuntimeItemData를 초기화하고,
// 플레이어가 상호작용 키(F)를 누르면 Interact()가 호출되어
// InventorySubsystem에 아이템을 추가한 뒤 자신을 Destroy합니다.
// =========================================================================================

#include "Item/PickableItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MyCharacter.h"

// 충돌 구체와 아이템 3D 메시를 생성하고 기본값을 설정합니다.
APickableItem::APickableItem()
{
    // 위치가 고정된 정적 오브젝트이므로 불필요한 Tick을 비활성화합니다.
    PrimaryActorTick.bCanEverTick = false;

    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    RootComponent = SphereComp;
    SphereComp->SetSphereRadius(50.0f);

    // 플레이어가 이동을 막히지 않고 상호작용 범위에 들어올 수 있도록 Overlap 전용으로 설정합니다.
    SphereComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);

    // 충돌 처리는 SphereComp가 담당하므로 메시의 충돌을 비활성화합니다.
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// 월드에 스폰된 직후 DataTable에서 아이템 데이터를 읽어 RuntimeItemData를 초기화합니다.
void APickableItem::BeginPlay()
{
    Super::BeginPlay();

    // ItemRowName으로 DataTable 행을 검색해 런타임 데이터를 채웁니다.
    if (ItemDataTable && !ItemRowName.IsNone())
    {
        FItemData* FoundData = ItemDataTable->FindRow<FItemData>(ItemRowName, TEXT("PickableItemLookup"));
        if (FoundData)
        {
            RuntimeItemData = *FoundData;

            // 에디터에서 설정한 DropQuantity로 수량을 덮어씁니다.
            RuntimeItemData.Quantity = DropQuantity;
        }
    }
}

// 플레이어가 상호작용할 때 호출되어 서버에 아이템 획득을 요청합니다.
// 실제 인벤토리 추가 및 액터 제거는 서버(ServerPickItem)에서 처리합니다.
void APickableItem::Interact(AActor* Interactor)
{
    // DataTable 원본이 아닌 BeginPlay에서 초기화한 RuntimeItemData를 사용합니다.
    if (RuntimeItemData.Quantity <= 0 || RuntimeItemData.ItemIcon == nullptr) return;

    AMyCharacter* MyChar = Cast<AMyCharacter>(Interactor);
    if (!MyChar) return;

    // 서버 RPC를 통해 서버에서 인벤토리 추가 및 액터 제거를 처리합니다.
    MyChar->ServerPickItem(this);
}
