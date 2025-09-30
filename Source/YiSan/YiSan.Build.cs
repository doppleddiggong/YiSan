// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class YiSan : ModuleRules
{
	public YiSan(ReadOnlyTargetRules Target) : base(Target)
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
			"YiSan",
			"YiSan/Variant_Platforming",
			"YiSan/Variant_Platforming/Animation",
			"YiSan/Variant_Combat",
			"YiSan/Variant_Combat/AI",
			"YiSan/Variant_Combat/Animation",
			"YiSan/Variant_Combat/Gameplay",
			"YiSan/Variant_Combat/Interfaces",
			"YiSan/Variant_Combat/UI",
			"YiSan/Variant_SideScrolling",
			"YiSan/Variant_SideScrolling/AI",
			"YiSan/Variant_SideScrolling/Gameplay",
			"YiSan/Variant_SideScrolling/Interfaces",
			"YiSan/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
