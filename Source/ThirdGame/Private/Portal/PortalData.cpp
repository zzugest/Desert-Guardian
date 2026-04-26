// =========================================================================================
// PortalData.cpp
//
// [파일 역할]
// 포탈 설정 데이터를 담는 FPortalData 구조체와 EPortalType 열거형의 구현 파일입니다.
// 순수 데이터 컨테이너이므로 별도의 cpp 로직이 없습니다.
// 실제 포탈 데이터는 에디터에서 DataTable 에셋으로 작성되며,
// MapPortal이 BeginPlay·InteractWithPortal 시 행(Row)을 읽어 사용합니다.
//
// EPortalType 종류:
//   - LevelTransition   : 확인창을 띄운 뒤 다른 레벨로 전환합니다.
//   - SameLevelTeleport : 같은 레벨 내 태그 액터 위치로 즉시 이동합니다.
// =========================================================================================

#include "Portal/PortalData.h"
