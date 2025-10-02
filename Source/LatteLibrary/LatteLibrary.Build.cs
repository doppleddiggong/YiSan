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
			
			"HTTP",
			"Json",
			"JsonUtilities",

			"Voice",     
			"AudioMixer" 
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoffeeLibrary",

			"EnhancedInput",
			"Niagara",

			"WebSockets",
			"AudioCapture",    
			"AudioCaptureCore",
			"AudioPlatformConfiguration",
			"DeveloperSettings"
		});
		
		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "Character", "Public"),
			Path.Combine(ModuleDirectory, "Common", "Public"),
			Path.Combine(ModuleDirectory, "Data", "Public"),
			Path.Combine(ModuleDirectory, "Manager", "Public"),
			Path.Combine(ModuleDirectory, "Network", "Public"),
		});
	}
}
