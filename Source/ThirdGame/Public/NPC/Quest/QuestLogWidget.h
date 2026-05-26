// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestLogWidget.generated.h"


// 전방 선언
class UTextBlock;
class UVerticalBox;
class UButton;
class UQuestComponent;

/**
 * 
 */
UCLASS()
class THIRDGAME_API UQuestLogWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // ��۱� �˶��� �︱ ������ �ؽ�Ʈ�� ���ΰ�ħ�� �Լ��Դϴ�.
    UFUNCTION()
    void UpdateQuestLog();

public:
    
    UPROPERTY(meta = (BindWidget))
    class URichTextBlock* QuestListText;

    // 퀘스트 텍스트와 자동이동 버튼을 세로로 묶는 컨테이너입니다.
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* QuestBackground;

    // 클릭하면 현재 Hunt 퀘스트 목표 위치로 자동이동을 시작하는 버튼입니다.
    UPROPERTY(meta = (BindWidget))
    UButton* AutoMoveButton;

    // 자동이동 버튼 클릭 콜백입니다.
    UFUNCTION()
    void OnAutoMoveClicked();

private:
    // 이 캐릭터의 퀘스트 컴포넌트를 캐시합니다.
    UPROPERTY()
    UQuestComponent* CachedQuestComp;
};
