// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class RN4UE4 : ModuleRules
{
	public RN4UE4(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RakNet", "Replicas", "PhysX", "APEX" });

		MinFilesUsingPrecompiledHeaderOverride = 1;
		bFasterWithoutUnity = false;
	}
}
