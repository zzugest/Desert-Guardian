#include "AnimNotify/AnimNotify_AttackHit.h"
#include "MyCharacter.h" 
#include "CombatComponent.h" 

void UAnimNotify_AttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	//  애니메이션이 이 프레임을 지나갔는지 확인
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("1. 애니메이션 노티파이 실행됨!"));

	if (MeshComp && MeshComp->GetOwner())
	{
		AMyCharacter* OwnerChar = Cast<AMyCharacter>(MeshComp->GetOwner());
		if (OwnerChar)
		{
			// 2. 캐릭터를 정상적으로 찾았는지 확인!
			//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("2. 내 캐릭터(AMyCharacter) 찾기 성공!"));

			if (OwnerChar->CombatComp) // (유저님의 CombatComponent 변수 이름)
			{
				// 3. 컴포넌트까지 완벽하게 찾았는지 확인!
				//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("3. CombatComp 찾음! 데미지 함수 호출 명령 내림!"));
				OwnerChar->CombatComp->ExecuteAttackHit(AttackID);
			}
			else
			{
				//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("에러: 내 캐릭터 안에 CombatComp가 비어있습니다(Null)!"));
			}
		}
		else
		{
			//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("에러: 몽타주를 재생 중인 주인이 AMyCharacter가 아닙니다!"));
		}
	}
}