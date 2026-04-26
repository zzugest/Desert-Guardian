// =========================================================================================
// ShopNPC.cpp
//
// [파일 역할]
// 플레이어가 상호작용하면 상점 UI를 화면에 열어 판매 기능을 제공하는 NPC 클래스입니다.
// DataTable과 SaleItemRowNames로 판매 아이템 목록을 런타임에 구성해 ShopWidget에 전달합니다.
// =========================================================================================

#include "NPC/Shop/ShopNPC.h"
#include "NPC/Shop/ShopComponent.h"
#include "NPC/Shop/ShopWidget.h"
#include "Blueprint/UserWidget.h"
#include "MyCharacter.h"
#include "Engine/DataTable.h"

// 상점 컴포넌트를 생성합니다. Tick은 불필요하므로 비활성화합니다.
AShopNPC::AShopNPC()
{
    PrimaryActorTick.bCanEverTick = false;

    ShopComp = CreateDefaultSubobject<UShopComponent>(TEXT("ShopComponent"));
}

// 플레이어 컨트롤러를 받아 상점 UI를 생성하고 DataTable에서 판매 아이템을 채워 초기화합니다.
void AShopNPC::OpenShop(APlayerController* PlayerController)
{
    if (!ShopUIClass || !PlayerController) return;

    UShopWidget* ShopWidget = CreateWidget<UShopWidget>(PlayerController, ShopUIClass);
    if (!ShopWidget) return;

    // SaleItemRowNames 목록을 순회해 DataTable에서 실제 FItemData를 찾아옵니다.
    TArray<FItemData> RealSaleItems;
    if (ItemDataTable)
    {
        for (FName RowName : SaleItemRowNames)
        {
            FItemData* FoundItem = ItemDataTable->FindRow<FItemData>(RowName, TEXT("Shop Item Lookup"));
            if (FoundItem)
            {
                RealSaleItems.Add(*FoundItem);
            }
        }
    }

    // 판매 목록을 ShopWidget에 전달해 그리드를 채우고 화면에 표시합니다.
    ShopWidget->InitShop(RealSaleItems);
    ShopWidget->AddToViewport();
}

// 플레이어가 상호작용 키(F)를 누르면 상점을 엽니다.
void AShopNPC::InteractWithPlayer(AMyCharacter* PlayerCharacter)
{
    if (!PlayerCharacter) return;

    APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
    if (!PC) return;

    OpenShop(PC);
}
