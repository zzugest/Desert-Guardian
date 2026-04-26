#include "Skill/SkillWindowWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Skill/SkillSubsystem.h"
#include "Skill/SkillListEntryWidget.h"
#include "Components/Button.h"

// SkillWindowWidget.cpp
// Purpose:
//   - ��ų ��� UI(��ų â) ����.
//   - SkillSubsystem�� ������ ���̺��� ��ȸ�� ��ų ī����� �������� �����Ͽ� WrapBox�� ��ġ.
// Key behaviors:
//   - RefreshUI: SkillContainer�� SkillEntryClass�� ��ȿ�ϸ� ���� �׸��� ���� �� ���������̺��� ��� ��ų�� ������� ����.
// Safety notes:
//   - GetGameInstance(), SkillSubsystem, SkillDataTable�� null üũ �ʿ�.
//   - �뷮 ���� ���� �� �����ս� ������ ������ ��(����¡ �Ǵ� ����ȭ ����).

void USkillWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	
	if (Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &USkillWindowWidget::OnCloseButtonClicked);
	}
}

void USkillWindowWidget::NativeDestruct()
{
	// [�߰�] UISubsystem�� ���� �����ٰ� �����մϴ�.
	/*if (UGameInstance* GI = GetGameInstance())
	{
		if (UUISubsystem* UISys = GI->GetSubsystem<UUISubsystem>())
		{
			UISys->ReportUIClosed(this);
		}
	}*/

	Super::NativeDestruct();
}

void USkillWindowWidget::RefreshUI()
{
	// �⺻���� ������ ��ȿ�� �˻�: ��ų ����Ʈ �����̳ʿ� ��Ʈ�� Ŭ���� �ʿ�
	if (!SkillContainer || !SkillEntryClass) return;

	// ���� ����� ��� ���� �׸���
	SkillContainer->ClearChildren();

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
	// SkillSubsystem�� ���������̺��� ��ȿ�ؾ� ��ų ��� ����
	if (SkillSys && SkillSys->GetSkillDataTable())
	{
		// ���������̺��� ��� RowName(��ų ID) ����� ������
		TArray<FName> AllSkillIDs = SkillSys->GetSkillDataTable()->GetRowNames();

		const int32 NumColumns = 3; // 3열 균등 배치
		int32 Index = 0;

		for (const FName& SkillID : AllSkillIDs)
		{
			USkillListEntryWidget* NewEntry = CreateWidget<USkillListEntryWidget>(this, SkillEntryClass);

			if (NewEntry)
			{
				NewEntry->UpdateUI(SkillID);

				// 행(Row) = Index / 3, 열(Column) = Index % 3 으로 3열 그리드 배치
				int32 Row    = Index / NumColumns;
				int32 Column = Index % NumColumns;

				UUniformGridSlot* GridSlot = SkillContainer->AddChildToUniformGrid(NewEntry, Row, Column);
				if (GridSlot)
				{
					GridSlot->SetHorizontalAlignment(HAlign_Fill);
					GridSlot->SetVerticalAlignment(VAlign_Fill);
				}

				Index++;
			}
		}
	}
}


void USkillWindowWidget::OnCloseButtonClicked()
{
	// "�� ������!" ��� ���� �Ҹ�Ĩ�ϴ� (�̺�Ʈ ���).
	// �׷��� �� �Ҹ��� �� ����̴�(AddDynamic �ߴ�) MyCharacter�� �Լ��� ����˴ϴ�.
	if (OnWindowClosed.IsBound())
	{
		OnWindowClosed.Broadcast();
	}
}