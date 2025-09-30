// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class LatteLibrary : ModuleRules
{
	public LatteLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"CoffeeLibrary"
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
			Path.Combine(ModuleDirectory, "Common", "Public"),
			Path.Combine(ModuleDirectory, "Character", "Public"),	
			Path.Combine(ModuleDirectory, "Data", "Public"),	
			Path.Combine(ModuleDirectory, "Manager", "Public"),	
			Path.Combine(ModuleDirectory, "Network", "Public"),	
		});
	}
}
