// =========================================================================================
// InventorySubsystem.h
//
// [역할 요약]
// 게임 전역에서 플레이어의 인벤토리 데이터를 보관하고 관리하며, 아이템 획득 및 슬롯 간 위치 교환 등의 핵심 기능을 제공하는 서브시스템 헤더입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemData.h" 
#include "InventorySubsystem.generated.h"

// 인벤토리 데이터에 변동이 생겼을 때 UI 화면 갱신 등을 지시하기 위해 호출되는 델리게이트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdatedDelegate);

UCLASS()
class THIRDGAME_API UInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	// 서브시스템 생성 시 인벤토리의 기본 용량(Capacity)을 설정합니다.
	UInventorySubsystem();

	// 런타임 초기화 시 인덱스 오류를 막기 위해 인벤토리 배열을 빈 칸(더미)으로 미리 세팅해 둡니다.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 인벤토리에 새 아이템을 추가하며, 스택 가능 여부 및 여유 공간을 검사해 성공 여부를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(const FItemData& NewItem);

	// 현재 인벤토리에 보관 중인 모든 아이템 데이터가 담겨 있는 핵심 배열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FItemData> Content;

	// 인벤토리가 수용할 수 있는 최대 아이템 칸(슬롯) 개수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Capacity = 20;

	// 인벤토리 내용물이 변경될 때마다 화면 UI 새로고침을 유도하기 위해 알림을 통보합니다.
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdatedDelegate OnInventoryUpdated;

	// 드래그 앤 드롭 조작 등을 통해 두 개 개별 인벤토리 슬롯의 내부 데이터 위치를 서로 맞바꿉니다.
	void SwapItems(int32 IndexA, int32 IndexB);
};