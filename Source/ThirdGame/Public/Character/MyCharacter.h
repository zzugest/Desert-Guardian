// =========================================================================================
// MyCharacter.h
//
// [���� ���]
// �÷��̾ ���� �����ϴ� ���� ĳ���� Ŭ������ ����Դϴ�.
// ī�޶�, ����, ��ų �ý��ۺ��� �Է� Ű ���ε�, �ִϸ��̼� �� UI ���� ���� �Ѱ��ϴ� �����̳� ������ �����մϴ�.
// =========================================================================================

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


UCLASS()
class THIRDGAME_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// QuestLogWidget은 WBP_HUDRoot의 자식으로 통합되었습니다.

	

public:
	virtual void Tick(float DeltaTime) override;

	// �÷��̾��� ���� Ű �Է��� ���� ������ �����մϴ�.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ĳ������ ���� ������ �����մϴ�.
	virtual void Jump() override;

	// �ȱ�, ���� �� ĳ������ �̵� ü�谡 ����� �� ȣ��˴ϴ�.
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	// ������ ���� �� �ٴڿ� �������� �� ȣ��˴ϴ�.
	virtual void Landed(const FHitResult& Hit) override;

	// =============================================================
	// [������Ʈ] �ٽ� ��� �и�
	// =============================================================

	// ȭ��(ī�޶�)�� ĳ���͸� ���� �Ÿ����� ����ٴϰ� ���ִ� �������Դϴ�.
	// 미니맵 캡처 및 위젯을 관리하는 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minimap")
	UMinimapComponent* MinimapComp;

	// PlayerHUD와 MinimapWidget을 하나로 묶는 루트 HUD 위젯입니다.
	// 에디터에서 WBP_HUDRoot 블루프린트를 할당합니다.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UHUDRootWidget> HUDRootClass;


	// 생성된 루트 HUD 인스턴스입니다. 컷신 등에서 이 하나만 숨기면 됩니다.
	UPROPERTY()
	UHUDRootWidget* HUDRoot;

	// 하위 호환용 포인터 — HUDRoot 생성 후 자식에서 꺼내어 할당합니다.
	// CombatComponent 등 기존 코드가 그대로 동작합니다.
	UPROPERTY()
	UMinimapWidget* MinimapWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* CameraBoom;

	// �÷��̾��� �þ߸� ���������� �������ϴ� ī�޶� ���Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* FollowCamera;

	// ����� �� �Ҹ�ǰ�̳� ��ų�� ������ ����� �� �ְ� ���� ������ ������Ʈ�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UQuickSlotComponent* QuickSlotComp;

	// ĳ������ ���� ����Ŭ, �޺� Ÿ�� ���� �� ������ ó���� �����ϴ� ���� ������Ʈ�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UCombatComponent* CombatComp;

	// 서버 권위 인벤토리 데이터를 관리하는 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UInventoryComponent* InventoryComp;

	// =============================================================
	// [�Է�] ���� �׼� (Enhanced Input)
	// =============================================================

	// ��ü���� ���� Ű ���� ������ ���õ� ���ؽ�Ʈ �����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	// Ű����(WSAD) �̵� ���� �׼��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	// ���콺�� ���� ���� �� ȸ�� ���� �׼��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	// ���� �߻� ���� �׼��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;

	// �⺻ ���� ���� ���� �׼��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAction;

	// NPC���� ��ȭ, ������ �ݱ� �� ȯ�� ���� �׼��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InteractAction;

	// �κ��丮 â(����)�� ���� �ݴ� ���� �׼��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InventoryAction;

	// �޸���(������Ʈ) Ȱ��ȭ ���� �׼��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MagicAttackAction;

	// �Է� �̺�Ʈ�� �����Ͽ� �̵� ó���� �����մϴ�.
	void Move(const FInputActionValue& Value);

	// �Է� �̺�Ʈ�� �����Ͽ� ī�޶� ������ �����ϴ�.
	void Look(const FInputActionValue& Value);

	// �Է� �̺�Ʈ�� �����Ͽ� ���� ������ Ʈ�����մϴ�.
	void Attack(const FInputActionValue& Value);

	// ============================================================
	// [네트워크] 공격 동기화 RPC
	// ============================================================

	// 클라이언트 → 서버: 포탈 텔레포트 요청. 서버 권위로 캐릭터를 목적지로 이동시킵니다.
	UFUNCTION(Server, Reliable)
	void Server_TeleportTo(FVector DestLocation, FRotator DestRotation);

	// 클라이언트 → 서버: 포탈 이동 요청. 서브레벨 로드 후 텔레포트하고 이전 서브레벨을 언로드합니다.
	UFUNCTION(Server, Reliable)
	void Server_RequestPortalTravel(FName TargetSubLevelName, FName UnloadSubLevelName, FVector Dest, FRotator Rot);

	// 목적지 서브레벨 로드 완료 시 호출 — 텔레포트 실행 후 이전 서브레벨 언로드를 요청합니다.
	UFUNCTION()
	void OnPortalLevelLoaded();

	// 이전 서브레벨 언로드 완료 시 호출 — 현재는 별도 처리 없음.
	UFUNCTION()
	void OnPortalUnloadComplete();

	// 서버 → 해당 클라이언트 전용: 포탈 이동 후 이 클라이언트 로컬에서만 존 지오메트리를 로드/언로드합니다.
	// 다른 클라이언트에는 영향을 주지 않아 각자의 화면에서 자신의 존만 렌더링됩니다.
	UFUNCTION(Client, Reliable)
	void Client_UpdateZoneStreaming(FName ToLoad, FName ToUnload);

	// 클라이언트 로컬 존 로드 완료 콜백 — 현재는 별도 처리 없음.
	UFUNCTION()
	void OnClientZoneLoaded();

	// 클라이언트 로컬 존 언로드 완료 콜백 — 현재는 별도 처리 없음.
	UFUNCTION()
	void OnClientZoneUnloaded();

	// 클라이언트 → 서버: 공격 요청. SnapRotation은 공격 시작 시 캐릭터가 바라볼 방향입니다.
	UFUNCTION(Server, Reliable)
	void ServerRequestAttack(FRotator SnapRotation);

	// 서버 → 모든 클라이언트: 몽타주 재생 명령. SnapRotation으로 모든 화면에서 방향을 동기화합니다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayComboMontage(int32 ComboIndex, FRotator SnapRotation);

	// 서버 → 모든 클라이언트: 점프 공격 몽타주(Start 섹션)를 재생합니다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayJumpAttackMontage();

	// 서버 → 모든 클라이언트: 점프 공격 몽타주를 Loop 섹션으로 전환합니다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayJumpLoopAnim();

	// 서버 → 모든 클라이언트: 착지 시 점프 공격 몽타주를 End 섹션으로 전환하거나 중단합니다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayJumpLandingAnim();

	// 클라이언트 → 서버: 마법 공격 요청. 클라이언트의 로컬 타겟을 파라미터로 전달합니다.
	UFUNCTION(Server, Reliable)
	void ServerRequestMagicAttack(AActor* TargetActor);

	// 서버 → 모든 클라이언트: 마법 콤보 몽타주를 재생합니다. TargetLocation으로 VFX 위치를 전달합니다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayMagicMontage(int32 ComboIndex, FVector TargetLocation);

	// 클라이언트 → 서버: 달리기 시작 요청. 서버에서 MaxWalkSpeed를 변경합니다.
	UFUNCTION(Server, Reliable)
	void ServerStartSprint();

	// 클라이언트 → 서버: 달리기 중지 요청. 서버에서 MaxWalkSpeed를 복구합니다.
	UFUNCTION(Server, Reliable)
	void ServerStopSprint();

	// 서버 → 모든 클라이언트: SP 소진 등으로 서버가 달리기를 강제 중단할 때 사용합니다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastForceStopSprint();

	// 클라이언트 → 서버: 구르기 요청. 방향 섹션 이름을 파라미터로 전달합니다.
	UFUNCTION(Server, Reliable)
	void ServerRequestRoll(FName SectionName);

	// 서버 → 모든 클라이언트: 구르기 몽타주를 재생합니다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayRollMontage(FName SectionName);

	// 클라이언트 → 서버: 클라이언트 측 WeaponTrace 히트 위치를 전달해 서버에서 이펙트를 스폰합니다.
	// 공중 공격 시 서버의 SimulatedProxy 위치 오차로 트레이스가 빗나가는 경우를 보정합니다.
	UFUNCTION(Server, Unreliable)
	void ServerReportWeaponHit(FVector HitLocation, FRotator HitNormal);

	// 클라이언트 → 서버: 아이템 줍기 요청. 서버에서 유효성 검증 후 인벤토리에 추가하고 액터를 제거합니다.
	UFUNCTION(Server, Reliable)
	void ServerPickItem(class APickableItem* Item);

	// 클라이언트 → 서버: 상점 아이템 구매 요청. 서버에서 인벤토리에 추가합니다.
	UFUNCTION(Server, Reliable)
	void ServerBuyItem(FItemData Item);

	// 클라이언트 → 서버: 퀘스트 보상 아이템 지급 요청. 서버에서 인벤토리에 추가합니다.
	UFUNCTION(Server, Reliable)
	void ServerClaimQuestReward(FItemData Item);

	// 클라이언트 → 서버: 스킬 시전 요청. 서버에서 MP 차감 후 애니메이션을 재생하고 투사체를 생성합니다.
	UFUNCTION(Server, Reliable)
	void ServerCastSkill(FName SkillID);

	// 서버 → 시전자 클라이언트: 스킬 피격 시 데미지 텍스트를 시전자 화면에만 표시합니다.
	UFUNCTION(Client, Reliable)
	void ClientShowDamageText(FVector Location, float Damage);

	// 서버 → 모든 클라이언트: 스킬 몽타주를 재생합니다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlaySkillMontage(UAnimMontage* Montage);

	// �Է� �̺�Ʈ�� �����Ͽ� ������ ��ȣ�ۿ��� �����մϴ�.
	void Interact(const FInputActionValue& Value);

	// �Է� �̺�Ʈ�� �����Ͽ� �κ��丮�� ����մϴ�.
	void ToggleInventory(const FInputActionValue& Value);

	UFUNCTION()
	void ForceCloseInventory();


	// ��ų Ʈ���� �����ִ� UI â�� ���ų� �ݽ��ϴ�.
	void ToggleSkillWindow(const FInputActionValue& Value);

	UFUNCTION()
	void ForceCloseSkillWindow();



	// =============================================================
	// [UI] ȭ�� �� ����
	// =============================================================

	// HUDRoot 생성 후 자식에서 꺼내어 할당합니다.
	// CombatComponent 등 기존 코드가 그대로 동작합니다.
	UPROPERTY()
	UPlayerHUDWidget* PlayerHUD;

	// ����ǰ ȭ���� ������ �κ��丮 ���� Ŭ�����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UInventoryWidget> InventoryWidgetClass;

	// ȭ�鿡 ������ �κ��丮 ���� �ν��Ͻ��Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UInventoryWidget* InventoryWidget;

	// ���� ������ ���� �� �κ��丮 UI�� ���ΰ�ħ�ϴ� �ݹ��Դϴ�.
	UFUNCTION()
	void OnInventoryUpdated();

	// �ִϸ��̼� ���� ���� ���� ������ �̾�� ���� ��û�Ǵ� ó�� �Լ��Դϴ�.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ProcessComboCommand();

	// ���� ����� �Ÿ��� �־� FŰ�� ������ �� �ֿ� �� �ִ� �ٴ��� ������ �����Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class APickableItem* OverlappingItem;

	// ��ü�� Ư�� ������ �������� �� ��ħ�� ���۵��� �˸��� �ݹ��Դϴ�.
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	// ���� �ִ� ��ü�� ������ ��� �� ȣ��Ǵ� �ݹ��Դϴ�.
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	// =============================================================
	// [������ �Է�] 
	// =============================================================

	// 1�� ~ 5�� ���� ��� ����Ű �׼��Դϴ�.
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

	// ����Ű �Է� �߻� �� ������ ������ �ߵ���Ű�� �Լ����Դϴ�.
	void OnQuickSlot1(const FInputActionValue& Value);
	void OnQuickSlot2(const FInputActionValue& Value);
	void OnQuickSlot3(const FInputActionValue& Value);
	void OnQuickSlot4(const FInputActionValue& Value);
	void OnQuickSlot5(const FInputActionValue& Value);

	// ���޹��� �ε����� �ش��ϴ� ������ �������� �Ҹ�/����մϴ�.
	void ProcessQuickSlot(int32 SlotIndex);

	// =============================================================
	// [�μ� �ý���/������Ʈ]
	// =============================================================

	// ���� �� ��� ȹ�� �������� ���� ���� �̺�Ʈ�� ó���� ������Ʈ�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UMoneyComponent* MoneyComp;

	// Ư�� ��ų(Q, E, R)�� ���� �Ǵܰ� ��ٿ��� ������ ������Ʈ�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USkillComponent* SkillComp;

	// Q, E, R ��ų ����Ű ���� �׼��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SkillQAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* SkillEAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* SkillRAction;

	// �Է� �̺�Ʈ�� �����Ͽ� Ư�� ��ų�� �����մϴ�.
	void UseSkillQ(const FInputActionValue& Value);
	void UseSkillE(const FInputActionValue& Value);
	void UseSkillR(const FInputActionValue& Value);

	// =============================================================
	// [���� �±� / ��Ÿ ������] 
	// =============================================================

	// ĳ������ ���� ����(���� ��, ����, ȸ�� ��)�� �����ϱ� ���� �����̳��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FGameplayTagContainer ActionTags;

	// ĳ���Ϳ��� Ư�� ���� �±׸� �ο��ϰų� ����, �����ϴ� ��ƿ��Ƽ �Լ����Դϴ�.
	void AddStateTag(FName TagName);
	void RemoveStateTag(FName TagName);

	UFUNCTION(BlueprintPure, Category = "Tags")
	bool HasStateTag(FName TagName);

	// ��ų ���׷��̵� â�� ȣ���ϴ� �Է� �׼��Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* OpenSkillWindowAction;

	// ȭ�鿡 ǥ�õ� ��ų Ʈ�� ���� Ŭ�����Դϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> SkillWindowClass;

	// �����Ǿ� �����ϴ� ��ų Ʈ�� â UI�� �ν��Ͻ��Դϴ�.
	UPROPERTY()
	class USkillWindowWidget* SkillWindowWidget;

	// �����̳� ���� �������� ���ظ� �Ծ��� �� ȣ��Ǿ� ü���� ��� ����� �����մϴ�.
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// ������ ����Ʈ�� ���� ������ �޼��� ��������� �����ϴ� ������Ʈ�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	UQuestComponent* QuestComponent;

	// ĳ���Ͱ� ����(��������) �̵��� �ϰ� �ִ��� �����Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	// ���� ��带 �۵���Ű�ų� �Ϲ� �ȱ�� �����մϴ�.
	void StartSprint();
	void StopSprint();

	// ���� ���� ȭ�� ���� �켱 ���� ����� ã�� ������ �ִ� Ÿ���� ������Ʈ�Դϴ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTargetingComponent* TargetingComp;

	// ĳ������ �ڼ��� ������(����) �Ǵ� ��ȭ�� ���� ��ȯ�մϴ�.
	void EnterCombatStance();
	void ExitCombatStance();

	// ���� �ڼ��� ������ ���� �ð� ������Ű�� ���� Ÿ�̸� ��ü�Դϴ�.
	FTimerHandle CombatTimerHandle;

	// 전투 자세 여부를 모든 클라이언트에 복제합니다.
	// 다른 플레이어 화면에서 스트레이프(옆 이동) 애니메이션이 올바르게 재생되도록 합니다.
	UPROPERTY(ReplicatedUsing = OnRep_CombatStance, BlueprintReadOnly, Category = "Combat")
	bool bIsInCombatStance = false;

	// bIsInCombatStance 복제 수신 시 자동 호출 — 이동 방향 설정을 로컬에 적용합니다.
	UFUNCTION()
	void OnRep_CombatStance();

	// 가장 최근 공격 요청 시 계산한 스냅 방향 — 콤보 이어치기 Multicast에 재사용합니다.
	FRotator LastSnapRotation = FRotator::ZeroRotator;

	// ���� �Ǵ� �ǰ� �� ���� ��� ���¸� ������ �ð�(��)�Դϴ�.
	UPROPERTY(EditAnywhere, Category = "Combat")
	float CombatTimeout = 5.0f;

	// ȸ�� Ű�� ������ �� �ٴ��� ���� ���ϴ� �ִϸ��̼� ��Ÿ���Դϴ�.
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* CombatRollMontage;

	// �����Ⱑ ����Ǿ��� �� ȣ��Ǿ� �ڼ� �±� ���� ���½�ŵ�ϴ�.
	UFUNCTION()
	void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);


	// ���� ������ ������ ����(Wrapper) �Լ� ����
	void MagicAttack();


	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|UI")
	void OnSpawnDamageText(FVector HitLocation, float DamageAmount);

	// 이 캐릭터가 현재 속한 서브레벨 이름입니다. 마을(Persistent Level)에 있으면 None입니다.
	// 레퍼런스 카운팅에서 다른 플레이어의 존 위치를 확인할 때 사용합니다.
	UPROPERTY()
	FName CurrentZoneName = NAME_None;

private:
	// 포탈 이동 대기 정보: 서브레벨 로드 완료 후 텔레포트에 사용됩니다.
	FVector  PendingTravelLocation     = FVector::ZeroVector;
	FRotator PendingTravelRotation     = FRotator::ZeroRotator;
	FName    PendingUnloadSubLevelName = NAME_None;
	FName    PendingTargetSubLevelName = NAME_None;
};