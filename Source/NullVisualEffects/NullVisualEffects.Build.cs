// Copyright (C) Ronaldo Veloso. All Rights Reserved.

using UnrealBuildTool;

public class NullVisualEffects : ModuleRules
{
    public NullVisualEffects(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange
        (
            new string[]
            {
                ModuleDirectory + "/Public",
            }
        );


        PrivateIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory + "/Private",
            }
        );


        PublicDependencyModuleNames.AddRange
        (
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "RenderCore",
                "Renderer",
                "RHI",
            }
        );


        PrivateDependencyModuleNames.AddRange
        (
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "RenderCore",
                "Renderer",
                "RHI",
            }
        );


        DynamicallyLoadedModuleNames.AddRange
        (
            new string[]
            {
            }
        );
    }
}
