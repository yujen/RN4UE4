// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#ifndef ____RakNetTestTool__
#define ____RakNetTestTool__

#include "IRakNet.h"
#include "IPluginManager.h"
#include "ModuleManager.h"


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
    void OnMyToolMenu();
    
    
private:
    
    TSharedPtr<FExtender> Extender;
    
};


#endif /* defined(____RakNetTestTool__) */
