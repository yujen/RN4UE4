// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class RakNet : ModuleRules
	{
		public RakNet(TargetInfo Target)
		{
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
                    "RakNet/Public",
                }
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					// ... add other private include paths required here ...
                    //"RakNet/Private",
                }
                );

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
                    //"UnrealEd",
                    //"LevelEditor",
                    //"Slate",
					// ... add other public dependencies that you statically link with here ...                    
                    "RakNet",
                }
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
                    "Core",
                    "Engine",
                    "UnrealEd",
                    "LevelEditor",
                    "Slate",
					// ... add private dependencies that you statically link with here ...
                    //"RakNet",
                }
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}