#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemData.h"
#include "InventoryComponent.generated.h"

// ���� ����
class UInventorySubsystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THIRDGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

	// ����ý����� �������� ���� �Լ�
	UInventorySubsystem* GetInventorySubsystem();
};
