// =========================================================================================
// TalkData.h
//
// [역할 요약]
// 퀘스트가 없는 일반 NPC(마을 주민, 경비병 등)가 출력할 단순 대화(잡담, 세계관 설명 등)
// 원본 데이터를 데이터 테이블(엑셀)에 정의하기 위한 구조체입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "TalkData.generated.h"

// ==========================================================
// 일반 대화 원본 데이터 구조체
// ==========================================================
// 데이터 테이블에서 관리될 단일 NPC의 대사 명세 구조체입니다.
USTRUCT(BlueprintType)
struct FTalkData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // UI 대화창 상단에 표시될 NPC의 이름입니다. (예: "지나가는 경비병")
    // FString 대신 FText를 사용하여 향후 다국어 번역(Localization)에 대비합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talk Data")
    FText NPCName;

    // 플레이어가 '다음(Next)' 버튼을 누를 때마다 순차적으로 출력될 대사들의 목록입니다.
    // 배열의 크기가 곧 대화의 총 페이지 수가 됩니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talk Data")
    TArray<FText> Dialogues;

    // (선택 사항) 대화창 좌측/우측에 띄울 NPC의 얼굴(초상화) 이미지입니다.
    // 2D RPG 느낌을 내고 싶을 때 유용하며, 당장 필요 없다면 에디터에서 비워두시면 됩니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talk Data")
    class UTexture2D* NPCPortrait = nullptr;

    // (선택 사항) 대화가 출력될 때 재생될 음성(더빙) 파일 목록입니다.
    // 대사(Dialogues) 배열의 인덱스와 맞춰서 재생할 수 있습니다.
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talk Data")
    // TArray<class USoundBase*> VoiceCues;
};