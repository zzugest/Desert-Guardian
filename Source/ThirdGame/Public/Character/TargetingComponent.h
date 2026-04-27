// =========================================================================================
// TargetingComponent.h
//
// [���� ���]
// ī�޶� ������ �������� ȭ�� �߾ӿ� ���� ������ �þ߰� Ȯ���� ���� �ڵ����� �ĺ��Ͽ� ǥ������ �����ϴ� �ڵ� Ÿ���� ������Ʈ ����Դϴ�.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THIRDGAME_API UTargetingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTargetingComponent();

    // �� ������ Ÿ�� ��ĵ ������ �ݺ� �����մϴ�.
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ���� �ý��ۿ� ���� �ֿ켱 ���� ���� ������� ������ ���� ��ü�Դϴ�.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
    AActor* CurrentTarget;

    // 카메라 전방으로 구체를 스윕할 최대 거리입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    float TraceDistance = 3000.0f;

    // 전방 스윕 구체의 반경입니다. 클수록 옆에 있는 적도 감지합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    float SweepRadius = 500.0f;

    // true로 설정하면 PIE에서 타겟팅 범위를 시각적으로 확인할 수 있습니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Debug")
    bool bShowDebug = false;

private:
    // ī�޶� ���氢(����)�� ���ü�(���� Ʈ���̽�)�� �˻��Ͽ� ȭ�� �߾ӿ� ���� ������ ���� �����س��ϴ�.
    void FindTarget();
};