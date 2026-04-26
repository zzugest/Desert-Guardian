// =========================================================================================
// InventoryComponent.cpp
//
// [파일 역할]
// 플레이어(폰)에 부착되어 인벤토리 관련 입력 및 이벤트 처리를 담당하는 컴포넌트입니다.
// 실제 아이템 데이터는 이 컴포넌트가 아닌 인벤토리 서브시스템(InventorySubsystem)에서 관리합니다.
// =========================================================================================

#include "Inventory/InventoryComponent.h"
#include "Inventory/InventorySubsystem.h"
#include "Kismet/GameplayStatics.h"

// 인벤토리 관련 로직에는 Tick이 필요하지 않으므로 비활성화합니다.
UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// 인벤토리 아이템 배열 데이터를 관리하는 서브시스템 레퍼런스를 반환합니다.
UInventorySubsystem* UInventoryComponent::GetInventorySubsystem()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return nullptr;

	return GI->GetSubsystem<UInventorySubsystem>();
}

// 현재 추가 초기화 로직이 없습니다.
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}
