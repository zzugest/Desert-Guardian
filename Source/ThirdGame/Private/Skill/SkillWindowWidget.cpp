#include "Skill/SkillWindowWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Skill/SkillSubsystem.h"
#include "Skill/SkillListEntryWidget.h"
#include "Components/Button.h"

// =========================================================================================
// SkillWindowWidget.cpp
//
// [파일 역할]
// 스킬 목록 UI(스킬 창) 위젯입니다.
// SkillSubsystem의 데이터테이블을 조회해 스킬 카드들을 3열 그리드로 생성하여 WrapBox에 배치합니다.
// =========================================================================================

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
	Super::NativeDestruct();
}

void USkillWindowWidget::RefreshUI()
{
	// 기본적인 의존성 유효성 검사: 스킬 리스트 컨테이너와 엔트리 클래스 필요
	if (!SkillContainer || !SkillEntryClass) return;

	// 기존 표시된 모든 스킬 그리드 초기화
	SkillContainer->ClearChildren();

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	USkillSubsystem* SkillSys = GI->GetSubsystem<USkillSubsystem>();
	// SkillSubsystem과 데이터테이블이 유효해야 스킬 목록 생성 가능
	if (SkillSys && SkillSys->GetSkillDataTable())
	{
		// 데이터테이블의 모든 RowName(스킬 ID) 목록을 가져옵니다.
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
	// 닫기 버튼 클릭 시 OnWindowClosed 델리게이트를 브로드캐스트합니다.
	// 바인딩된 MyCharacter의 함수에서 실제 창 닫기 처리를 수행합니다.
	if (OnWindowClosed.IsBound())
	{
		OnWindowClosed.Broadcast();
	}
}