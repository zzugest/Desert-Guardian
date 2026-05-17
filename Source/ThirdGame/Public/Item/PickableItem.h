// =========================================================================================
// PickableItem.h
//
// [역할 요약]
// 게임 월드 공간에 배치되어 플레이어가 직접 다가가 획득(루팅)할 수 있는 상호작용형 아이템 액터 헤더입니다.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemData.h" 
#include "PickableItem.generated.h"

class UDataTable;

UCLASS()
class THIRDGAME_API APickableItem : public AActor
{
	GENERATED_BODY()

public:
	APickableItem();

protected:
	virtual void BeginPlay() override;

public:
	// 월드에 표시될 아이템의 시각적 3D 모델 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ItemMesh;

	// 플레이어 캐릭터와의 겹침(접근)을 감지하는 구형 영역 판정 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* SphereComp;

	//  원본 데이터를 찾아올 아이템 데이터 테이블 에셋 지정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	UDataTable* ItemDataTable;

	//  데이터 테이블 안에서 이 아이템이 참조할 행 이름 (예: "Potion_HP_01")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FName ItemRowName;

	//  게임 시작(BeginPlay) 시 테이블에서 읽어와 채워 넣을 실제 런타임 데이터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Data")
	FItemData RuntimeItemData;

	//  맵에 배치할 때, 기본 1개가 아니라 여러 개(예: 포션 5개 묶음)를 떨구고 싶을 때 조절할 수량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 DropQuantity = 1;

	// 플레이어가 상호작용 조작 키를 입력했을 때 호출되어 아이템 수납 처리를 수행합니다.
	UFUNCTION()
	void Interact(AActor* Interactor);
};