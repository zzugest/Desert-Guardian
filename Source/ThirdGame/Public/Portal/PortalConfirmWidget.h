#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PortalData.h" // 데이터 구조체 인클루드
#include "PortalConfirmWidget.generated.h"

class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPortalConfirmAccepted);

UCLASS()
class THIRDGAME_API UPortalConfirmWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//  외부(포탈)에서 데이터를 넘겨받아 UI 글씨를 세팅하는 함수
	UFUNCTION(BlueprintCallable, Category = "Portal UI")
	void InitConfirmUI(const FPortalData& PortalData);

	// 수락 버튼 클릭 시 MapPortal에 알리는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Portal UI")
	FOnPortalConfirmAccepted OnAccepted;

protected:
	// 위젯이 화면에 생성될 때 버튼 클릭 이벤트를 연결해줄 곳
	virtual void NativeConstruct() override;

	// --------------------------------------------------
	// 에디터(블루프린트)와 이름을 똑같이 맞춰야 하는 변수들
	// --------------------------------------------------
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Question;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Accept;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Decline;

	// 버튼을 눌렀을 때 실행될 함수들
	UFUNCTION()
	void OnAcceptClicked();

	UFUNCTION()
	void OnDeclineClicked();

};