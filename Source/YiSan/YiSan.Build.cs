// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class YiSan : ModuleRules
{
	public YiSan(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore",
			"UMG",
			"Slate",
			"SlateCore",
			"MediaAssets",
			"RenderCore",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks"
		});
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"HTTP",
			"Json",
			"JsonUtilities",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"WebSockets",
		});
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"OnlineSubsystem",
			"OnlineSubsystemSteam",
		});
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Voice",     
			"AudioMixer" 
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AudioCapture",
			"AudioCaptureCore",
			"AudioPlatformConfiguration",
			
			"DeveloperSettings"
		});
		
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoffeeLibrary",
			"LatteLibrary"
		});
		
		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "Character", "Public"),
			Path.Combine(ModuleDirectory, "Environment", "Public"),
			Path.Combine(ModuleDirectory, "Loading", "Public"),
			Path.Combine(ModuleDirectory, "Mass", "Public"),
			Path.Combine(ModuleDirectory, "Network", "Public"),
			Path.Combine(ModuleDirectory, "NPC", "Public"),
			Path.Combine(ModuleDirectory, "UI", "Public"),
			Path.Combine(ModuleDirectory, "Voice", "Public"),
		});
	}
}

