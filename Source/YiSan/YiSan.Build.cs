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
			"CoffeeLibrary",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"Niagara",
			"HTTP",
			"Json",
			"JsonUtilities",
			"WebSockets",
			"DeveloperSettings"
		});

		PublicIncludePaths.AddRange(new string[] {
			"YiSan",
			"YiSan/Template",
		});
	}
}
