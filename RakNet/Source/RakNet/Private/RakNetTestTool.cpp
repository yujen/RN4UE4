// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "RakNetPrivatePCH.h"
#include "RakNetTestTool.h"
#include "LevelEditor.h"
//#include "MultiBox/MultiBoxBuilder.h"


#define LOCTEXT_NAMESPACE "RakNetTestTool"



IMPLEMENT_MODULE(FRakNetTestTool, RakNetTestTool);
DEFINE_LOG_CATEGORY(LogRakNetTestTool);


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
		LOCTEXT("PingServer", "PingServer"),
		LOCTEXT("PingServerTip", "PingServerTip"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FRakNetTestTool::OnRakNetPingServer)));
	MenuBuilder.AddMenuEntry(
		LOCTEXT("PingClient", "PingClient"),
		LOCTEXT("PingClientTip", "PingClientTip"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FRakNetTestTool::OnRakNetPingClient)));
	MenuBuilder.EndSection();
}

void FRakNetTestTool::OnRakNetPingServer()
{
	UE_LOG(LogRakNetTestTool, Log, TEXT("OnRakNetPingServer"));

}

void FRakNetTestTool::OnRakNetPingClient()
{
	UE_LOG(LogRakNetTestTool, Log, TEXT("OnRakNetPingClient"));

}


#undef LOCTEXT_NAMESPACE
