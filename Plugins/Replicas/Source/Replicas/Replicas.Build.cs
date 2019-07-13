// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Replicas : ModuleRules
{
	public Replicas(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				"Replicas/Public",
                "Replicas/Include"
				// ... add public include paths required here ...
			}
			);

        PrivateIncludePaths.AddRange(
			new string[] {
                "Replicas/Private",
				"Replicas/Source"
				// ... add other private include paths required here ...
			}
			);


        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Projects",
                "RakNet"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"RakNet"
				// ... add private dependencies that you statically link with here ...	
			}
			);
	}
}
