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
			Path.Combine(ModuleDirectory, "Features", "Public"),	
			Path.Combine(ModuleDirectory, "Shared", "Public"),	
		});
	}
}
