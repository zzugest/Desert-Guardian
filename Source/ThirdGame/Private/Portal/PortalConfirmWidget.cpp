// =========================================================================================
// PortalConfirmWidget.cpp
//
// [파일 역할]
// 레벨 전환 포탈 상호작용 시 "○○(으)로 이동하시겠습니까?" 확인창을 표시하는 위젯입니다.
// 수락 클릭 시 입력·마우스 상태를 복구한 뒤 OpenLevel로 목적지 레벨을 불러옵니다.
// 거절 클릭 시 창을 닫고 플레이어 조작을 원래대로 돌려줍니다.
// =========================================================================================

#include "Portal/PortalConfirmWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

// 위젯 생성 시 수락·거절 버튼에 클릭 이벤트를 바인딩합니다.
void UPortalConfirmWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_Accept)
    {
        Btn_Accept->OnClicked.AddDynamic(this, &UPortalConfirmWidget::OnAcceptClicked);
    }
    if (Btn_Decline)
    {
        Btn_Decline->OnClicked.AddDynamic(this, &UPortalConfirmWidget::OnDeclineClicked);
    }
}

// MapPortal이 이 함수로 포탈 데이터를 주입합니다.
// 목적지 이름을 캐시하고, 확인 텍스트를 "○○(으)로 이동하시겠습니까?" 형식으로 설정합니다.
void UPortalConfirmWidget::InitConfirmUI(const FPortalData& PortalData)
{
    // 수락 시 OpenLevel에 사용할 레벨 이름을 미리 저장해둡니다.
    SavedMapName = PortalData.TargetLevelName;

    if (Text_Question)
    {
        FString QuestionStr = PortalData.LevelKoreaName + TEXT("(으)로 이동하시겠습니까?");
        Text_Question->SetText(FText::FromString(QuestionStr));
    }
}

// 수락 버튼: 입력·마우스 상태를 게임 모드로 복구하고 목적지 레벨로 전환합니다.
void UPortalConfirmWidget::OnAcceptClicked()
{
    if (SavedMapName.IsNone()) return;

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        // UI 전용 모드를 해제하고 마우스 커서를 숨깁니다.
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;

        // MapPortal이 DisableInput으로 잠근 이동 입력을 복구합니다.
        ACharacter* MyChar = Cast<ACharacter>(PC->GetPawn());
        if (MyChar)
        {
            MyChar->EnableInput(PC);
        }
    }

    // 포탈 이동 로직은 Persistent Level + 서브레벨 방식으로 교체 예정입니다.
    // (현재 임시 비활성화)
}

// 거절 버튼: 확인창을 닫고 플레이어 입력과 마우스 상태를 원래대로 복구합니다.
void UPortalConfirmWidget::OnDeclineClicked()
{
    RemoveFromParent();

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;

        ACharacter* MyChar = Cast<ACharacter>(PC->GetPawn());
        if (MyChar)
        {
            MyChar->EnableInput(PC);
        }
    }
}
