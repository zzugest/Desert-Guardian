// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class ThirdGame : ModuleRules
{
	public ThirdGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Slate", "SlateCore", "GameplayTags", "Niagara", "AIModule", "GameplayTasks", "NavigationSystem", "LevelSequence", "MovieScene" });

		

		PublicIncludePaths.AddRange(new string[] {
			// 캐릭터 관련
			"ThirdGame/Public/Character",
			// 아이템 / 인벤토리 / 퀵슬롯
			"ThirdGame/Public/Item",
			"ThirdGame/Public/Inventory",
			"ThirdGame/Public/QuickSlot",
			// 스킬
			"ThirdGame/Public/Skill",
			// 적 (일반 + 보스 서브폴더)
			"ThirdGame/Public/Enemy",
			"ThirdGame/Public/Enemy/Boss",
			// NPC (대화 / 퀘스트 / 상점)
			"ThirdGame/Public/NPC",
			"ThirdGame/Public/NPC/Quest",
			"ThirdGame/Public/NPC/Shop",
			// 포탈
			"ThirdGame/Public/Portal",
			// 글로벌 UI (경고 위젯, 데미지 타입 등)
			"ThirdGame/Public/GlobalUI",
			// 애님 노티파이
			"ThirdGame/Public/AnimNotify",
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
