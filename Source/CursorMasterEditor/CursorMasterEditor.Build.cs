using UnrealBuildTool;

public class CursorMasterEditor : ModuleRules
{
	public CursorMasterEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CursorMaster"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"AssetTools",
				"EditorFramework",
				"PropertyEditor",
				"Slate",
				"SlateCore",
				"ApplicationCore",
				"DesktopPlatform",
				"ImageWrapper",
				"InputCore",
				"UnrealEd",
				"Projects",
				"WorkspaceMenuStructure"
			}
		);
	}
}
