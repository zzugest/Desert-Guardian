// =========================================================================================
// WarningWidget.cpp
//
// [파일 역할]
// 화면에 표시되는 경고 메시지 위젯입니다.
// WarningSubsystem::ShowWarning에서 호출되며, 텍스트를 갱신하고
// Anim_FadeInOut 애니메이션을 처음부터 재생해 자연스러운 토스트 알림을 구현합니다.
// =========================================================================================

#include "GlobalUI/WarningWidget.h"
#include "Components/TextBlock.h"

// 경고 텍스트를 갱신하고 페이드 인/아웃 애니메이션을 처음부터 재생합니다.
void UWarningWidget::ShowWarningMessage(FText NewMessage)
{
	// 텍스트 블록에 새 메시지를 설정합니다.
	if (Txt_WarningMessage)
	{
		Txt_WarningMessage->SetText(NewMessage);
	}

	// 애니메이션을 0초부터 다시 재생해 이전 메시지가 사라지는 도중에도 즉시 갱신됩니다.
	if (Anim_FadeInOut)
	{
		PlayAnimation(Anim_FadeInOut, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
	}
}
