#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemData.h"
#include "InventoryComponent.generated.h"

class UInventorySubsystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THIRDGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

	// InventorySubsystem 레퍼런스를 반환하는 내부 헬퍼 함수
	UInventorySubsystem* GetInventorySubsystem();

public:
	// ============================
	// [복제] 서버 권위 인벤토리 데이터
	// ============================

	// 서버가 소유하는 실제 인벤토리 배열입니다. 소유 클라이언트에만 복제됩니다.
	UPROPERTY(ReplicatedUsing=OnRep_Inventory, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FItemData> Content;

	// 인벤토리 최대 슬롯 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Capacity = 20;

	// 복제할 변수를 등록합니다.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Content 복제 수신 시 호출 — InventorySubsystem에 동기화하고 UI 갱신을 알립니다.
	UFUNCTION()
	void OnRep_Inventory();

	// ============================
	// [Server RPC] 아이템 조작 요청
	// ============================

	// 클라이언트 → 서버: 아이템 추가 요청 (줍기, 퀘스트 보상, 상점 구매)
	UFUNCTION(Server, Reliable)
	void ServerAddItem(FItemData NewItem);

	// 클라이언트 → 서버: 두 슬롯 간 위치 교환 요청 (드래그 앤 드롭)
	UFUNCTION(Server, Reliable)
	void ServerSwapItems(int32 IndexA, int32 IndexB);

	// 클라이언트 → 서버: 퀵슬롯 아이템 사용 요청 (GUID로 아이템 식별, SlotIndex는 퀵슬롯 UI용)
	UFUNCTION(Server, Reliable)
	void ServerUseItem(FGuid ItemID, int32 QuickSlotIndex);

	// ============================
	// [내부 헬퍼] 서버에서만 호출
	// ============================

	// 실제 아이템 추가 로직 — 서버에서만 호출합니다. 성공 시 true 반환.
	bool AddItemInternal(const FItemData& NewItem);

	// 실제 슬롯 교환 로직 — 서버에서만 호출합니다.
	void SwapItemsInternal(int32 IndexA, int32 IndexB);
};
