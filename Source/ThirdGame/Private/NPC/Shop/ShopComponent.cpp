// =========================================================================================
// ShopComponent.cpp
//
// [파일 역할]
// ShopNPC에 부착되는 액터 컴포넌트입니다.
// 현재는 ShopNPC가 직접 상점 로직을 처리하므로 별도 기능 없이
// 컴포넌트 존재 여부로 "상점 NPC임"을 식별하는 마커 역할을 합니다.
// =========================================================================================

#include "NPC/Shop/ShopComponent.h"

UShopComponent::UShopComponent()
{
    // 정적 컴포넌트이므로 Tick을 비활성화합니다.
    PrimaryComponentTick.bCanEverTick = false;
}
