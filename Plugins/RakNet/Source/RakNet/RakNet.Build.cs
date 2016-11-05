// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RakNet : ModuleRules
{
    public RakNet(TargetInfo Target)
    {

        PublicIncludePaths.AddRange(new string[]
        {
            "RakNet/Public"
				// ... add public include paths required here ...
		});


        PrivateIncludePaths.AddRange(new string[]
        {
            "RakNet/Private",
				// ... add other private include paths required here ...
		});


        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            //"Engine",
				// ... add other public dependencies that you statically link with here ...
		});


        PrivateDependencyModuleNames.AddRange(new string[]
        {
            //"Core",
            "CoreUObject",
            "Engine",
				
				// ... add private dependencies that you statically link with here ...	
		});


        DynamicallyLoadedModuleNames.AddRange(new string[]
        {
				// ... add any modules that your module loads dynamically here ...
		});
        /*
        if (Target.Type == TargetRules.TargetType.Editor)
        {
            // These modules are only needed for RakNetTestTool
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "LevelEditor",
                "UnrealEd",
                "Slate"
            });

        }
        */

    }
}
