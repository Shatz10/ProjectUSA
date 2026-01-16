// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectUSA : ModuleRules
{
	public ProjectUSA(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange (new string[] {"ProjectUSA"});

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "UMG" , "LevelSequence", 
"MovieScene", "AIModule", "Niagara", "PhysicsCore"});

        PublicDependencyModuleNames.AddRange(new string[] {
            "GameFeatures",
            "GameplayAbilities",
            "GameplayTasks",
            "GameplayTags"
        });

// To use Windows32 features
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add("user32.lib");
        }

    }
}
