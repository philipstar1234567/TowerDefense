// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TowerDefenceProject : ModuleRules
{
	public TowerDefenceProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TowerDefenceProject",
			"TowerDefenceProject/Variant_Platforming",
			"TowerDefenceProject/Variant_Platforming/Animation",
			"TowerDefenceProject/Variant_Combat",
			"TowerDefenceProject/Variant_Combat/AI",
			"TowerDefenceProject/Variant_Combat/Animation",
			"TowerDefenceProject/Variant_Combat/Gameplay",
			"TowerDefenceProject/Variant_Combat/Interfaces",
			"TowerDefenceProject/Variant_Combat/UI",
			"TowerDefenceProject/Variant_SideScrolling",
			"TowerDefenceProject/Variant_SideScrolling/AI",
			"TowerDefenceProject/Variant_SideScrolling/Gameplay",
			"TowerDefenceProject/Variant_SideScrolling/Interfaces",
			"TowerDefenceProject/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
