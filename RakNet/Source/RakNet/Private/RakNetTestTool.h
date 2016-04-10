// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#ifndef ____RakNetTestTool__
#define ____RakNetTestTool__

#include "IRakNet.h"
#include "IPluginManager.h"
#include "ModuleManager.h"


DECLARE_LOG_CATEGORY_EXTERN(LogRakNetTestTool, Log, All);


class FRakNetTestTool : public IRakNet
{
public:
    
    /** Accessor for the module interface */
    static FRakNetTestTool& Get()
    {
        return FModuleManager::Get().GetModuleChecked<FRakNetTestTool>(TEXT("RakNetTestTool"));
    }
    
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    
    
    void OnWindowMenuExtension(FMenuBuilder& MenuBuilder);

    void OnRakNetPingServer();
	void OnRakNetPingClient();
    
private:
    
    TSharedPtr<FExtender> Extender;

	bool enableServer;
	RakNet::RakPeerInterface* server;
    
};


#endif /* defined(____RakNetTestTool__) */
