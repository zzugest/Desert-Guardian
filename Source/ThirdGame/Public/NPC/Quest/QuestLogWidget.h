// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestLogWidget.generated.h"


// ���� ����
class UTextBlock;
class UBorder;
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

    // 퀘스트 텍스트 뒤에 깔리는 반투명 배경 Border
    UPROPERTY(meta = (BindWidget))
    UBorder* QuestBackground;

private:
    // �� ĳ������ ����Ʈ ���� �����͸� ����� �� ����
    UPROPERTY()
    UQuestComponent* CachedQuestComp;
};
