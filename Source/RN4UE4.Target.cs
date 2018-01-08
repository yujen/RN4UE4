// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class RN4UE4Target : TargetRules
{
	public RN4UE4Target(TargetInfo Target) : base (Target)
	{
		Type = TargetType.Game;
		
		ExtraModuleNames.Add("RN4UE4");
	}
}
