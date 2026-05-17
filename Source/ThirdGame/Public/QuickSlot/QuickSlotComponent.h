// =========================================================================================
// QuickSlotComponent.h
//
// [역할 요약]
// 캐릭터에 부착되어 인벤토리의 아이템을 단축키(퀵슬롯)에 바인딩하고, 실제 사용 및 UI와의 연동 역할을 수행하는 로컬 컴포넌트 헤더입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemData.h"
#include "QuickSlotComponent.generated.h"

class UQuickSlotSubsystem;

// 퀵슬롯 목록 데이터가 변경되었을 때 화면(UI) 업데이트를 지시하기 위한 알림 델리게이트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuickSlotUpdated);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THIRDGAME_API UQuickSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuickSlotComponent();

protected:
	// 컴포넌트 초기화 단계에서 전역 서브시스템과 연동하여 기존 퀵슬롯 데이터를 복원합니다.
	virtual void BeginPlay() override;

	// 실제 데이터를 보관하는 전역 퀵슬롯 서브시스템 인스턴스를 확보하여 반환합니다.
	UQuickSlotSubsystem* GetQuickSlotSubsystem();

public:
	// =========================================================
	// 데이터 및 알림 (Events)
	// =========================================================

	// 캐릭터가 로컬에서 보관 및 관리하는 현재 퀵슬롯 아이템 목록 배열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot")
	TArray<FItemData> Slots;

	// 슬롯이 갱신되었을 때 연결된 UI 위젯들을 새로고침하도록 통보하는 이벤트 인스턴스입니다.
	UPROPERTY(BlueprintAssignable, Category = "QuickSlot")
	FOnQuickSlotUpdated OnQuickSlotUpdated;

	// =========================================================
	// 주요 기능 API
	// =========================================================

	// 지정된 퀵슬롯 인덱스 위치에 새로운 아이템 데이터를 등록(덮어쓰기)하고 갱신을 지시합니다.
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SetSlot(int32 SlotIndex, const FItemData& NewItem);

	// 지정된 퀵슬롯 인덱스에 등록되어 있는 대상 아이템의 소모 및 발동 로직을 실행합니다.
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void UseSlot(int32 SlotIndex);
};