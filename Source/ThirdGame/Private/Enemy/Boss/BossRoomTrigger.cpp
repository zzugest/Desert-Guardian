// =========================================================================================
// BossRoomTrigger.cpp
//
// [파일 역할]
// 보스 방 입구에 배치되어 플레이어가 진입하면 보스 인트로 컷신을 재생하는 트리거 액터입니다.
//
// [컷신 처리 흐름]
// 1. 플레이어 진입 감지 → LevelSequencePlayer로 컷신 재생 시작
// 2. HUDRoot 를 Hidden으로 전환 → PlayerHUD·MinimapWidget 전체 숨김
// 3. State.Cutscene 태그 추가 → TargetingComponent 비활성화
// 4. 컷신 종료 → HUDRoot 복원 + State.Cutscene 태그 제거 + 버프 UI 갱신
// =========================================================================================

#include "BossRoomTrigger.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Kismet/GameplayStatics.h"
#include "Character/MyCharacter.h"
#include "Character/HUDRootWidget.h"
#include "Skill/SkillComponent.h"

// 충돌 박스를 Trigger 프로파일로 설정하고 오버랩 이벤트를 바인딩합니다.
ABossRoomTrigger::ABossRoomTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	CollisionBox->SetCollisionProfileName(TEXT("Trigger"));
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ABossRoomTrigger::OnOverlapBegin);
}

// 플레이어가 진입하면 컷신을 재생하고 HUD를 숨깁니다. 이미 재생한 경우 무시합니다.
void ABossRoomTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 이미 컷신을 재생했거나 에셋이 없으면 무시합니다.
	if (bHasPlayed || !BossIntroSequence) return;

	ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
	if (PlayerChar && PlayerChar->IsLocallyControlled())
	{
		bHasPlayed = true;

		ALevelSequenceActor* OutActor = nullptr;
		FMovieSceneSequencePlaybackSettings Settings;
		Settings.bHidePlayer          = true;
		Settings.bDisableMovementInput = true;
		Settings.bDisableLookAtInput   = true;

		ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
			GetWorld(), BossIntroSequence, Settings, OutActor);

		if (SequencePlayer)
		{
			CachedPlayer = Cast<AMyCharacter>(PlayerChar);

			// 컷신 중에는 HUDRoot 전체를 숨겨 모든 자식 HUD를 한 번에 비활성화합니다.
			if (CachedPlayer && CachedPlayer->HUDRoot)
			{
				CachedPlayer->HUDRoot->SetVisibility(ESlateVisibility::Hidden);
			}

			// State.Cutscene 태그로 TargetingComponent와 기타 시스템을 비활성화합니다.
			if (CachedPlayer)
			{
				CachedPlayer->AddStateTag("State.Cutscene");
			}

			SequencePlayer->Play();

			// 컷신 종료 콜백을 등록합니다.
			SequencePlayer->OnFinished.AddDynamic(this, &ABossRoomTrigger::OnCutsceneFinished);
		}
	}
}

// 컷신 종료 시 HUD를 복원하고 컷신 태그를 제거한 뒤 버프 UI를 갱신합니다.
void ABossRoomTrigger::OnCutsceneFinished()
{
	if (!CachedPlayer) return;

	// 컷신이 끝나면 HUDRoot 전체를 다시 표시합니다.
	if (CachedPlayer->HUDRoot)
	{
		CachedPlayer->HUDRoot->SetVisibility(ESlateVisibility::Visible);
	}

	// State.Cutscene 태그를 제거해 타겟팅 및 기타 시스템을 재활성화합니다.
	CachedPlayer->RemoveStateTag("State.Cutscene");

	// 컷신 경과 시간 동안 버프 지속 시간이 흘렀으므로 버프 UI를 갱신합니다.
	if (USkillComponent* SkillComp = CachedPlayer->FindComponentByClass<USkillComponent>())
	{
		SkillComp->OnBuffListUpdated.Broadcast();
	}
}
