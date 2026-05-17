#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPC/BaseNPC.h"
#include "ShopNPC.generated.h"

// ShopWidget을 알기 위해 전방 선언
class UShopWidget;

UCLASS()
class THIRDGAME_API AShopNPC : public ABaseNPC
{
    GENERATED_BODY()

public:
    AShopNPC();

protected:
    // 상점 컴포넌트 (진열대)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
    class UShopComponent* ShopComp;
   
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop Data")
    class UDataTable* ItemDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop Data")
    TArray<FName> SaleItemRowNames;


    // 에디터에서 'WBP_Shop'을 넣을 변수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UShopWidget> ShopUIClass;

public:
    // 상호작용 함수 (플레이어가 F키를 눌렀을 때 호출될 예정)
    UFUNCTION(BlueprintCallable, Category = "Shop")
    virtual void InteractWithPlayer(AMyCharacter* PlayerCharacter) override;

    void OpenShop(APlayerController* PlayerController);
};