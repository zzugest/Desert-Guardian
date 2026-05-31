
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MoneyComponent.h"
#include "NPC/Shop/ShopNPC.h"
#include "Skill/SkillComponent.h"
#include "GameplayTagContainer.h"
#include "MyCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UPlayerHUDWidget;
class USpringArmComponent;
class UCameraComponent;
class UInventoryComponent;
class UQuickSlotComponent;
class UCombatComponent;
class UQuestComponent;
class UTargetingComponent;
class UUserWidget;
class UMinimapComponent;
class UMinimapWidget;
class UHUDRootWidget;
class UAutoMoveComponent;
class UCursorOptionComponent;
class UPostProcessComponent;

UCLASS()
class THIRDGAME_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Jump() override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	virtual void Landed(const FHitResult& Hit) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minimap")
	UMinimapComponent* MinimapComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Death")
	UPostProcessComponent* DeathPostProcess;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UHUDRootWidget> HUDRootClass;

	UPROPERTY()
	UHUDRootWidget* HUDRoot;

	UPROPERTY()
	UMinimapWidget* MinimapWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UQuickSlotComponent* QuickSlotComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UCombatComponent* CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UInventoryComponent* InventoryComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UAutoMoveComponent* AutoMoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UCursorOptionComponent* CursorOptionComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InventoryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MagicAttackAction;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Attack(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_TeleportTo(FVector DestLocation, FRotator DestRotation);

	UFUNCTION(Server, Reliable)
	void Server_RequestPortalTravel(FName TargetSubLevelName, FName UnloadSubLevelName, FVector Dest, FRotator Rot);

	UFUNCTION()
	void OnPortalLevelLoaded();

	UFUNCTION(Server, Reliable)
	void Server_RequestGroupPortalTravel(FName TargetSubLevelName, FName UnloadSubLevelName, FName SourceZoneName, FVector Dest, FRotator Rot);

	UFUNCTION()
	void OnGroupPortalLevelLoaded();

	UFUNCTION()
	void OnPortalUnloadComplete();

	UFUNCTION(Client, Reliable)
	void Client_UpdateZoneStreaming(FName ToLoad, FName ToUnload);

	UFUNCTION()
	void OnClientZoneLoaded();

	UFUNCTION()
	void OnClientZoneUnloaded();

	UFUNCTION(Server, Reliable)
	void ServerRequestAttack(FRotator SnapRotation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayComboMontage(int32 ComboIndex, FRotator SnapRotation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayJumpAttackMontage();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayJumpLoopAnim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayJumpLandingAnim();

	UFUNCTION(Server, Reliable)
	void ServerRequestMagicAttack(AActor* TargetActor);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayMagicMontage(int32 ComboIndex, FVector TargetLocation);

	UFUNCTION(Server, Reliable)
	void ServerStartSprint();

	UFUNCTION(Server, Reliable)
	void ServerStopSprint();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastForceStopSprint();

	UFUNCTION(Server, Reliable)
	void ServerRequestRoll(FName SectionName);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayRollMontage(FName SectionName);

	UFUNCTION(Server, Reliable)
	void Server_ReportHitEnemy(AActor* HitEnemy, FVector HitLocation, FRotator HitNormal, FName AttackID);

	UFUNCTION(Server, Reliable)
	void Server_ReportMagicHit(const TArray<AActor*>& HitEnemies, FName MagicAttackID);

	UFUNCTION(Server, Reliable)
	void ServerPickItem(class APickableItem* Item);

	UFUNCTION(Server, Reliable)
	void ServerBuyItem(FItemData Item);

	UFUNCTION(Server, Reliable)
	void ServerClaimQuestReward(FItemData Item);

	UFUNCTION(Server, Reliable)
	void ServerCastSkill(FName SkillID);

	UFUNCTION(Client, Reliable)
	void ClientShowDamageText(FVector Location, float Damage);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlaySkillMontage(UAnimMontage* Montage);

	void Interact(const FInputActionValue& Value);

	void ToggleInventory(const FInputActionValue& Value);

	UFUNCTION()
	void ForceCloseInventory();

	void ToggleSkillWindow(const FInputActionValue& Value);

	UFUNCTION()
	void ForceCloseSkillWindow();

	UPROPERTY()
	UPlayerHUDWidget* PlayerHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UInventoryWidget> InventoryWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UInventoryWidget* InventoryWidget;

	UFUNCTION()
	void OnInventoryUpdated();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ProcessComboCommand();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class APickableItem* OverlappingItem;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* QuickSlot1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* QuickSlot2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* QuickSlot3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* QuickSlot4Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* QuickSlot5Action;

	void OnQuickSlot1(const FInputActionValue& Value);
	void OnQuickSlot2(const FInputActionValue& Value);
	void OnQuickSlot3(const FInputActionValue& Value);
	void OnQuickSlot4(const FInputActionValue& Value);
	void OnQuickSlot5(const FInputActionValue& Value);

	void ProcessQuickSlot(int32 SlotIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UMoneyComponent* MoneyComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USkillComponent* SkillComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SkillQAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* SkillEAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* SkillRAction;

	void UseSkillQ(const FInputActionValue& Value);
	void UseSkillE(const FInputActionValue& Value);
	void UseSkillR(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FGameplayTagContainer ActionTags;

	void AddStateTag(FName TagName);
	void RemoveStateTag(FName TagName);

	UFUNCTION(BlueprintPure, Category = "Tags")
	bool HasStateTag(FName TagName);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* OpenSkillWindowAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> SkillWindowClass;

	UPROPERTY()
	class USkillWindowWidget* SkillWindowWidget;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	UQuestComponent* QuestComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	void StartSprint();
	void StopSprint();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTargetingComponent* TargetingComp;

	void EnterCombatStance();
	void ExitCombatStance();

	FTimerHandle CombatTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_CombatStance, BlueprintReadOnly, Category = "Combat")
	bool bIsInCombatStance = false;

	UFUNCTION()
	void OnRep_CombatStance();

	FRotator LastSnapRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float CombatTimeout = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* CombatRollMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|HitReaction")
	UAnimMontage* KnockbackFrontMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|HitReaction")
	UAnimMontage* KnockbackBackMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|HitReaction")
	UAnimMontage* KnockbackLeftMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|HitReaction")
	UAnimMontage* KnockbackRightMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|HitReaction")
	UAnimMontage* LaunchMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|HitReaction")
	UAnimMontage* AirLaunchMontage;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayLaunchReaction(FRotator FaceRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAirLaunchReaction(FRotator FaceRotation);

	UFUNCTION()
	void OnAirLaunchMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditAnywhere, Category = "Combat|HitReaction")
	UAnimMontage* AirKnockbackFrontMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|HitReaction")
	UAnimMontage* AirKnockbackBackMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|HitReaction")
	UAnimMontage* AirKnockbackLeftMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|HitReaction")
	UAnimMontage* AirKnockbackRightMontage;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitReaction(UAnimMontage* Montage, bool bIsAirHit);

	UFUNCTION()
	void OnHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditAnywhere, Category = "Combat")
	UDataTable* RespawnDataTable;

	void Die();

	UFUNCTION()
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void Respawn();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeathMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Respawn(FVector Location, FRotator Rotation);

	UFUNCTION()
	void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void MagicAttack();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|UI")
	void OnSpawnDamageText(FVector HitLocation, float DamageAmount);

	UPROPERTY()
	FName CurrentZoneName = NAME_None;

private:
	FVector  PendingTravelLocation     = FVector::ZeroVector;
	FRotator PendingTravelRotation     = FRotator::ZeroRotator;
	FName    PendingUnloadSubLevelName = NAME_None;
	FName    PendingTargetSubLevelName = NAME_None;

	FVector  PendingGroupLocation      = FVector::ZeroVector;
	FRotator PendingGroupRotation      = FRotator::ZeroRotator;
	FName    PendingGroupTargetSub     = NAME_None;
	FName    PendingGroupUnloadSub     = NAME_None;
	FName    PendingGroupSourceZone    = NAME_None;
};
