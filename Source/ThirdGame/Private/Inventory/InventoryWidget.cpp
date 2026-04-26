// =========================================================================================
// InventoryWidget.cpp
//
// [파일 역할]
// 전체 인벤토리 창을 화면에 표시하는 루트 UI 위젯 클래스입니다.
// TileView(타일 뷰)를 사용해 여러 슬롯을 격자 형태로 배치하고,
// 창이 열리고 닫힐 때 UISubsystem에 상태를 알려 입력 모드를 자동으로 전환합니다.
//
// [성능 최적화]
// UpdateUI는 Capacity가 바뀔 때만 TileView 항목을 재생성하고,
// 이후 호출부터는 기존 UInventoryItemObject의 데이터만 갱신해
// 매 프레임 GC 부담을 없앱니다.
// =========================================================================================

#include "Inventory/InventoryWidget.h"
#include "Inventory/InventorySlotWidget.h"
#include "Components/TileView.h"
#include "Inventory/InventoryItemObject.h"
#include "UISubsystem.h"
#include "Kismet/GameplayStatics.h"

// 위젯이 화면에 추가될 때 닫기 버튼 이벤트를 바인딩하고 UISubsystem에 열림을 알립니다.
void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_Close)
    {
        Btn_Close->OnClicked.AddDynamic(this, &UInventoryWidget::OnCloseButtonClicked);
    }

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UUISubsystem* UISys = GI->GetSubsystem<UUISubsystem>();
    if (!UISys) return;

    // UISubsystem이 열린 위젯 목록을 관리해 마우스 커서 표시 및 입력 모드를 자동 전환합니다.
    UISys->ReportUIOpened(this);
}

// 위젯이 소멸할 때 UISubsystem에 닫힘을 알려 입력 모드를 복구합니다.
void UInventoryWidget::NativeDestruct()
{
    Super::NativeDestruct();

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UUISubsystem* UISys = GI->GetSubsystem<UUISubsystem>();
    if (!UISys) return;

    UISys->ReportUIClosed(this);
}

// 서브시스템의 최신 인벤토리 데이터를 받아 TileView 슬롯을 갱신합니다.
void UInventoryWidget::UpdateUI(const TArray<FItemData>& Content, int32 Capacity)
{
    if (!InventoryTileView) return;

    // Capacity가 바뀌었을 때만 ItemObject를 재생성합니다.
    // 이후 호출에서는 기존 객체의 데이터만 갱신해 불필요한 GC 부담을 방지합니다.
    bool bNeedsRebuild = (CachedItemObjects.Num() != Capacity);

    if (bNeedsRebuild)
    {
        InventoryTileView->ClearListItems();
        CachedItemObjects.SetNum(Capacity);
    }

    for (int32 i = 0; i < Capacity; i++)
    {
        if (!CachedItemObjects[i])
        {
            CachedItemObjects[i] = NewObject<UInventoryItemObject>(this);
            CachedItemObjects[i]->Index = i;
        }

        CachedItemObjects[i]->ItemData = Content.IsValidIndex(i) ? Content[i] : FItemData();

        if (bNeedsRebuild)
        {
            InventoryTileView->AddItem(CachedItemObjects[i]);
        }
    }

    // 재생성이 아닌 경우 RegenerateAllEntries()로 NativeOnListItemObjectSet 재호출을 보장합니다.
    if (!bNeedsRebuild)
    {
        InventoryTileView->RegenerateAllEntries();
    }
}

// 닫기 버튼 클릭 시 OnWindowClosed 델리게이트를 브로드캐스트해 MyCharacter가 창을 닫도록 합니다.
void UInventoryWidget::OnCloseButtonClicked()
{
    if (OnWindowClosed.IsBound())
    {
        OnWindowClosed.Broadcast();
    }
}
