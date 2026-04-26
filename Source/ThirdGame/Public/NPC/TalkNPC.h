#pragma once

#include "CoreMinimal.h"
#include "NPC/BaseNPC.h" // 경로가 다르면 원석님의 BaseNPC 경로로 맞춰주세요.
#include "TalkNPC.generated.h"

// 위젯과 데이터 테이블을 사용하기 위한 전방 선언
class UUserWidget;
class UDataTable;

UCLASS()
class THIRDGAME_API ATalkNPC : public ABaseNPC
{
    GENERATED_BODY()

public:
    ATalkNPC();

    // 부모(BaseNPC)의 상호작용 함수를 덮어씁니다. (플레이어가 F키를 눌렀을 때 실행됨)
    virtual void InteractWithPlayer(AMyCharacter* PlayerCharacter) override;

protected:
    // 일반 대사를 읽어올 엑셀 표(데이터 테이블)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talk UI")
    UDataTable* TalkDataTable;

    // 화면에 띄울 대화창 위젯 클래스 (블루프린트에서 할당할 예정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talk UI")
    TSubclassOf<UUserWidget> DialogueWidgetClass;

    // 이 NPC가 데이터 테이블의 어느 행(Row)을 읽을지 지정하는 이름표
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talk UI")
    FName NPCTalkRowName;
};