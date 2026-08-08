// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FTPShooter : ModuleRules
{
	public FTPShooter(ReadOnlyTargetRules Target) : base(Target)
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
			"FTPShooter",
			"FTPShooter/Variant_Platforming",
			"FTPShooter/Variant_Platforming/Animation",
			"FTPShooter/Variant_Combat",
			"FTPShooter/Variant_Combat/AI",
			"FTPShooter/Variant_Combat/Animation",
			"FTPShooter/Variant_Combat/Gameplay",
			"FTPShooter/Variant_Combat/Interfaces",
			"FTPShooter/Variant_Combat/UI",
			"FTPShooter/Variant_SideScrolling",
			"FTPShooter/Variant_SideScrolling/AI",
			"FTPShooter/Variant_SideScrolling/Gameplay",
			"FTPShooter/Variant_SideScrolling/Interfaces",
			"FTPShooter/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
