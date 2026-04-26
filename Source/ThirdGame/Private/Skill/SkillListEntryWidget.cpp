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

// SkillListEntryWidget.cpp
// Purpose:
//   - ��ų ���(����Ʈ)�� ���� ��Ʈ�� ����.
//   - ������ ���̺����� ��ų ������ �о� �̸�/�������� ǥ���ϰ� �巡�� ������ ó��.
// Key behaviors:
//   - UpdateUI: SkillID�� �޾� SkillSubsystem���� ������ �ε� �� UI ����.
//   - �巡��: ��Ŭ�� �巡�� �ν�, �巡�� �� DragOp�� SkillID�� ��� �̵���ų �� �ְ� ��.
// Safety notes:
//   - GetGameInstance()/SkillSubsystem/SkillData null üũ �ʿ�.

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
            // �̸��� ������ ����
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
    // ���� ��ư�� ���� �巡�� ���� ����
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void USkillListEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    // �巡�� ���۷��̼� ���� �� SkillID ž��
    USkillDragDropOp* DragOp = NewObject<USkillDragDropOp>();
    if (DragOp)
    {
        DragOp->SkillID = CurrentSkillID;

        // ���� ���־� ����(������ ǥ�ÿ�)
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