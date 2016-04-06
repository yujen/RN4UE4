// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "RakNetPrivatePCH.h"
#include "RakNetTestTool.h"
#include "LevelEditor.h"
//#include "MultiBox/MultiBoxBuilder.h"


#define LOCTEXT_NAMESPACE "RakNetTestTool"



IMPLEMENT_MODULE(FRakNetTestTool, RakNetTestTool);


void FRakNetTestTool::StartupModule()
{
	if (IsRunningCommandlet()) { return; }

	Extender = MakeShareable(new FExtender);
	Extender->AddMenuExtension(
		"General",
		EExtensionHook::After,
		NULL,
		FMenuExtensionDelegate::CreateRaw(this, &FRakNetTestTool::OnWindowMenuExtension)
		);
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(Extender);
}

void FRakNetTestTool::ShutdownModule()
{
	if (Extender.IsValid() && FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(Extender);
	}
}


void FRakNetTestTool::OnWindowMenuExtension(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("RakNetHook", LOCTEXT("RakNet", "RakNet"));
	MenuBuilder.AddMenuEntry(
		LOCTEXT("RakNetTestTool", "RakNetTestTool"),
		LOCTEXT("RakNetTestToolTip", "RakNetTestTool"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FRakNetTestTool::OnRakNetTestToolMenu)));
	MenuBuilder.EndSection();
}

void FRakNetTestTool::OnRakNetTestToolMenu()
{
}



#undef LOCTEXT_NAMESPACE
