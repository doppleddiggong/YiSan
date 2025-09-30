// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class CoffeeLibrary : ModuleRules
{
	public CoffeeLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
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
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd", 
				"EditorSubsystem"
			});
		}
		
		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "Actor", "Public"),
			Path.Combine(ModuleDirectory, "Core", "Public"),	
			Path.Combine(ModuleDirectory, "Common", "Public"),
			Path.Combine(ModuleDirectory, "Character", "Public"),	
			Path.Combine(ModuleDirectory, "Data", "Public"),	
			Path.Combine(ModuleDirectory, "Features", "Public"),	
			Path.Combine(ModuleDirectory, "Manager", "Public"),	
			Path.Combine(ModuleDirectory, "Network", "Public"),	
			Path.Combine(ModuleDirectory, "Shared", "Public"),	
		});
	}
}
