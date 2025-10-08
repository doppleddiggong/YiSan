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
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"EnhancedInput",
			"Niagara",
			
			"CoffeeLibrary",
		});
		
		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "Character", "Public"),
			Path.Combine(ModuleDirectory, "Common", "Public"),
			Path.Combine(ModuleDirectory, "Data", "Public"),
			Path.Combine(ModuleDirectory, "Manager", "Public"),
		});
	}
}
