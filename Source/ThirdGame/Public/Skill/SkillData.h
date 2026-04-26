// =========================================================================================
// SkillData.h
//
// [���� ���]
// ������ ���̺�(����) � ������ ���� ��ų�� ���� ����(�̸�, ����, ��ٿ�, ������ ��� ��) ��Ÿ������ ����ü�� �����ϴ� ����Դϴ�.
// =========================================================================================

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "Engine/Texture2D.h" 
#include "Animation/AnimMontage.h" 
#include "SkillData.generated.h"

class UNiagaraSystem;
class AProjectileBase;

// �ý��� �󿡼� ��ų�� �ֵ� ����(����, ����, ȸ�� ��)�� �з��ϰ� �����ϱ� ���� ������ �������Դϴ�.
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Attack      UMETA(DisplayName = "Attack"),
	Buff        UMETA(DisplayName = "Buff"),
	Heal        UMETA(DisplayName = "Heal"),
	Passive     UMETA(DisplayName = "Passive")
};

// ==========================================================
// ��ų ���� ������ ����ü
// ==========================================================
// ������ ���̺����� ������ ���� ��ų�� ���� �Ӽ�(��)���� �����ϴ� ����ü�Դϴ�.
USTRUCT(BlueprintType)
struct FSkillData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// UI 화면에 표시될 스킬의 표시용 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FText SkillName;

	// 툴팁 등 UI에 표시되는 한국어 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FText SkillKoreaName;

	// ��ų�� ����������, ���������� ���� �ǹ��ϴ� �Ӽ� �з��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	ESkillType SkillType = ESkillType::Attack;

	// ��ų�� ȿ���� ���� ���� ������ ���� �ؽ�Ʈ�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FText Description;

	// ��ųâ�̳� ������ UI�� ǥ�õ� ��ų�� ��ǥ ������ �̹����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	UTexture2D* Icon = nullptr;

	// ��ų ��� �� �ٽ� ����ϱ���� �䱸�Ǵ� ���� ���ð�(��)�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Cooldown = 5.0f;

	// ���� �� �ο��� ��ų�� ���, ��󿡰� ȿ���� �����Ǵ� ��ü �ð�(��)�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Duration = 0.0f;

	// ��ų ���(�ߵ�) �� �ﰢ������ �Ҹ�Ǵ� ĳ������ ���� �䱸���Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float ManaCost = 10.0f;

	// ��ų �ߵ� �� ĳ���Ͱ� ����ϰ� �� ���� �׼� ���(��Ÿ��)�Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	UAnimMontage* SkillMontage = nullptr;

	// 스킬 타입이 Attack일 때 기본 공격력에 곱해서 최종 피해량을 계산하는 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float DamageMultiplier = 1.0f;

	// 스킬 타입이 Buff일 때 올려줄 공격력 수치입니다. (툴팁의 {buff_amount} 플레이스홀더와 연동)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float BuffAmount = 0.0f;

	// ��ų ���� Ȥ�� ȿ�� ���� �߿� ��󿡰� ��½�ų �ð� ����Ʈ(���̾ư���) �����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	UNiagaraSystem* BuffEffect = nullptr;

	// ���̾ �� ���Ÿ� ����(����ü)�� ������ ��ų�� ���, ������ �߻�ü �� Ŭ�����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TSubclassOf<AProjectileBase> ProjectileClass = nullptr;
};