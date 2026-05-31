#include "Skill/SkillListEntryWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Skill/SkillSubsystem.h"
#include "Skill/SkillDragDropOp.h"
#include "Skill/SkillDragVisual.h"
#include "Input/Reply.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Character/CombatComponent.h"
#include "SkillTooltipWidget.h"

// =========================================================================================
// SkillListEntryWidget.cpp
//
// [파일 역할]
// 스킬 목록(리스트)의 각 항목 위젯입니다.
// 데이터 테이블에서 스킬 정보를 읽어 이름/아이콘을 표시하고 드래그 앤 드롭을 처리합니다.
// =========================================================================================

void USkillListEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 위젯 생성 시 딱 한 번만 CombatComponent를 찾아 캐싱합니다.
    // NativeOnMouseEnter에서 매번 FindComponentByClass를 호출하는 것을 방지합니다.
    ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (PlayerChar)
    {
        CachedCombatComp = PlayerChar->FindComponentByClass<UCombatComponent>();
    }
}

void USkillListEntryWidget::UpdateUI(FName SkillID)
{
    CurrentSkillID = SkillID;

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
    if (SkillSys)
    {
        FSkillData* Data = SkillSys->GetSkillData(SkillID);
        if (Data)
        {
            // 스킬 이름과 아이콘을 UI에 표시합니다.
            if (SkillName) SkillName->SetText(Data->SkillName);
            if (SkillIcon) SkillIcon->SetBrushFromTexture(Data->Icon);
        }
    }
}

void USkillListEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (CurrentSkillID.IsNone() || !SkillTooltipClass) return;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
	if (!SkillSys) return;

	FSkillData* Data = SkillSys->GetSkillData(CurrentSkillID);
	if (!Data) return;

	// NativeConstruct에서 캐싱해둔 포인터를 바로 사용합니다.
	float BaseAttackPower = CachedCombatComp ? CachedCombatComp->BaseAttackPower : 0.0f;

	USkillTooltipWidget* TooltipWidget = CreateWidget<USkillTooltipWidget>(this, SkillTooltipClass);
	if (TooltipWidget)
	{
		TooltipWidget->InitTooltip(*Data, BaseAttackPower);
		SetToolTip(TooltipWidget);
	}
}

FReply USkillListEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 왼쪽 버튼을 클릭 시 드래그 감지 시작
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void USkillListEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    // 드래그 오퍼레이션 생성 및 SkillID 탑재
    USkillDragDropOp* DragOp = NewObject<USkillDragDropOp>();
    if (DragOp)
    {
        DragOp->SkillID = CurrentSkillID;

        // 드래그 시각화 위젯 생성 (드래그 중 표시용)
        if (DragVisualClass)
        {
            USkillDragVisual* VisualWidget = CreateWidget<USkillDragVisual>(this, DragVisualClass);

            if (VisualWidget)
            {
                if (UGameInstance* GI = GetGameInstance())
                {
                    if (USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>())
                    {
                        FSkillData* Data = SkillSys->GetSkillData(CurrentSkillID);
                        if (Data)
                        {
                            VisualWidget->SetDragIcon(Data->Icon);
                        }
                    }
                }

                DragOp->DefaultDragVisual = VisualWidget;
            }
        }
        OutOperation = DragOp;
    }
}