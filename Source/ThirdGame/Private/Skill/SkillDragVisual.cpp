#include "Skill/SkillDragVisual.h"
#include "Components/Image.h"

// SkillDragVisual.cpp
// Purpose:
//   - 드래그 중 보여줄 시각적 위젯 구현(아이콘 등).
//   - 드래그 비주얼은 마우스 입력을 받지 않도록 HitTestInvisible로 설정.
// Safety notes:
//   - DragIconImage 포인터 null 체크.

void USkillDragVisual::SetDragIcon(UTexture2D* IconTexture)
{
    if (DragIconImage && IconTexture)
    {
        // 아이콘 텍스처를 브러시에 적용
        DragIconImage->SetBrushFromTexture(IconTexture);

        // 드래그 비주얼은 마우스/입력 히트 테스트 대상이 아니어야 함
        DragIconImage->SetVisibility(ESlateVisibility::HitTestInvisible);

        // 위젯 전체도 입력을 막아 따라다니는 동안 클릭 이벤트를 흘려보내도록 함
        this->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
}