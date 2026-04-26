#include "Skill/SkillSlotWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "Skill/SkillComponent.h"
#include "Skill/SkillSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Skill/SkillDragDropOp.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Skill/SkillDragVisual.h"
#include "Character/CombatComponent.h"
#include "SkillTooltipWidget.h"

// SkillSlotWidget.cpp
// Purpose:
//   - ���� ��ų ���� UI ����. ���Կ� �Ҵ�� ��ų�� ������/��ٿ��� ǥ���ϰ� �巡��/����� ����.
// Key behaviors:
//   - NativeConstruct: ������ ĳ������ SkillComponent ��ȸ, SkillSubsystem ��������Ʈ ����, �ʱ� UpdateSlotInfo ȣ��.
//   - UpdateSlotInfo: ���Կ� ������ SkillID�� ��ȸ�ϰ� ������/��ٿ� �ִ�ġ ����.
//   - NativeTick: ������ SkillComponent�� ���� ��Ÿ���� ǥ��(���α׷����� ����).
//   - �巡��/��� ����: ��ų �巡�� �� ���־� ����, ��� �� EquipSkill ȣ��.
// Safety notes:
//   - GetGameInstance()/SkillSubsystem/OwnerSkillComp ������ null �˻� �ʿ�.

void USkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 1) 플레이어의 SkillComponent / CombatComponent 캐싱
	ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (PlayerChar)
	{
		OwnerSkillComp  = PlayerChar->FindComponentByClass<USkillComponent>();
		OwnerCombatComp = PlayerChar->FindComponentByClass<UCombatComponent>();
	}

	// 2) SkillSubsystem ��������Ʈ ����: ���� ���� ����� UpdateSlotInfo�� ȣ��ǵ��� ��
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>())
		{
			SkillSys->OnSkillSlotUpdated.AddDynamic(this, &USkillSlotWidget::UpdateSlotInfo);
		}
	}

	// 3) �ʱ� UI ����
	UpdateSlotInfo();
}

void USkillSlotWidget::NativeDestruct()
{
	Super::NativeDestruct();

	// 위젯이 파괴될 때 SkillSubsystem 델리게이트 바인딩을 해제합니다.
	// 해제하지 않으면 위젯 재생성 시 중복 등록되어 UpdateSlotInfo가 여러 번 호출됩니다.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>())
		{
			SkillSys->OnSkillSlotUpdated.RemoveDynamic(this, &USkillSlotWidget::UpdateSlotInfo);
		}
	}
}

void USkillSlotWidget::UpdateSlotInfo()
{
	// ���� �˻�
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
	if (!SkillSys) return;

	// ���Կ� ���ε� ��ų ���̵� ��ȸ
	CurrentSkillID = SkillSys->GetSkillIDInSlot(SlotIndex);

	// 스킬이 없으면 UI 숨기고 툴팁도 제거
	if (CurrentSkillID.IsNone())
	{
		if (SkillIcon) SkillIcon->SetVisibility(ESlateVisibility::Hidden);
		if (CooldownBar) CooldownBar->SetVisibility(ESlateVisibility::Hidden);
		SetToolTip(nullptr);
		return;
	}

	// ���������̺����� ��ų ���� ��ȸ �� ������/��Ÿ�� ����
	FSkillData* Data = SkillSys->GetSkillData(CurrentSkillID);
	if (Data)
	{
		if (SkillIcon)
		{
			SkillIcon->SetBrushFromTexture(Data->Icon);
			SkillIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}

	// 쿨다운 바 초기화 (이후에 가려졌다가 쿨타임 중에는 보이게 됨)
	if (CooldownBar)
	{
		CooldownBar->SetPercent(0.0f);
		CooldownBar->SetVisibility(ESlateVisibility::Visible);
	}

	// 스킬이 등록되는 시점에 툴팁을 미리 세팅해둡니다.
	// NativeOnMouseEnter에서 세팅하면 UMG가 hover 처리를 시작한 뒤라 타이밍이 늦어
	// 첫 번째 hover에서는 툴팁이 뜨지 않는 문제가 생깁니다.
	UpdateTooltip();
}

void USkillSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// ���� ��Ÿ���� SkillComponent���� ��ȸ�Ͽ� ���α׷����ٿ� �ݿ�
	if (!OwnerSkillComp || CurrentSkillID.IsNone()) return;

	float RemainingTime = OwnerSkillComp->GetRemainingCooldown(CurrentSkillID);
	float MaxTime = OwnerSkillComp->GetMaxCooldown(CurrentSkillID);

	if (RemainingTime > 0.0f && MaxTime > 0.0f)
	{
		float Percent = RemainingTime / MaxTime;
		if (CooldownBar)
		{
			CooldownBar->SetPercent(Percent);
			CooldownBar->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		if (CooldownOverlay)
		{
			CooldownOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		if (CooldownText)
		{
			int32 IntTime = FMath::CeilToInt(RemainingTime);
			CooldownText->SetText(FText::AsNumber(IntTime));
			CooldownText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
	else
	{
		if (CooldownBar) CooldownBar->SetVisibility(ESlateVisibility::Hidden);
		if (CooldownOverlay) CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
		if (CooldownText) CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void USkillSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// �����Ϳ��� KeyName�� �����ϸ� ��� �ݿ�
	if (KeyText)
	{
		KeyText->SetText(KeyName);
	}
}

bool USkillSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// ��ӵ� ���۷��̼��� ��ų �巡������ �˻�
	USkillDragDropOp* DragOp = Cast<USkillDragDropOp>(InOperation);
	if (!DragOp) return false;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return false;

	USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
	if (!SkillSys) return false;

	// ���� ����� ��û: ����ý����� �ߺ�ó���� ��ε�ĳ��Ʈ�� ���
	SkillSys->EquipSkill(SlotIndex, DragOp->SkillID);

	return true;
}

FReply USkillSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// �� ������ �ƴϰ� ���� Ŭ���̸� �巡�� ���� Ʈ����
	if (!CurrentSkillID.IsNone() && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
		return Reply.NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void USkillSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	// �� �����̸� �巡�� �Ұ�
	if (CurrentSkillID.IsNone()) return;

	// �巡�� ���۷��̼� ���� �� SkillID ���
	USkillDragDropOp* DragOp = NewObject<USkillDragDropOp>();
	if (DragOp)
	{
		DragOp->SkillID = CurrentSkillID;

		// ���� ���־� ���� ����(�ɼ�)
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
				DragOp->Pivot = EDragPivot::MouseDown;
			}
		}

		OutOperation = DragOp;
	}
}

void USkillSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	// 마우스가 올라올 때 최신 BaseAttackPower 기준으로 툴팁을 갱신합니다.
	// (버프로 공격력이 바뀌어도 항상 현재 값을 반영)
	UpdateTooltip();
}

void USkillSlotWidget::UpdateTooltip()
{
	// 스킬이 없거나, 툴팁 클래스가 미설정이거나, CombatComponent가 없으면 툴팁 제거
	if (CurrentSkillID.IsNone() || !SkillTooltipClass || !OwnerCombatComp)
	{
		SetToolTip(nullptr);
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
	if (!SkillSys) return;

	FSkillData* Data = SkillSys->GetSkillData(CurrentSkillID);
	if (!Data) return;

	// 툴팁 위젯 생성 후 현재 공격력으로 초기화
	USkillTooltipWidget* TooltipWidget = CreateWidget<USkillTooltipWidget>(this, SkillTooltipClass);
	if (TooltipWidget)
	{
		TooltipWidget->InitTooltip(*Data, OwnerCombatComp->BaseAttackPower);
		SetToolTip(TooltipWidget);
	}
}