
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "Engine/Texture2D.h" 
#include "Animation/AnimMontage.h" 
#include "SkillData.generated.h"

class UNiagaraSystem;
class AProjectileBase;

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Attack      UMETA(DisplayName = "Attack"),
	Buff        UMETA(DisplayName = "Buff"),
	Heal        UMETA(DisplayName = "Heal"),
	Passive     UMETA(DisplayName = "Passive")
};

USTRUCT(BlueprintType)
struct FSkillData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FText SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FText SkillKoreaName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	ESkillType SkillType = ESkillType::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Cooldown = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float ManaCost = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	UAnimMontage* SkillMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float BuffAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	UNiagaraSystem* BuffEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TSubclassOf<AProjectileBase> ProjectileClass = nullptr;
};
