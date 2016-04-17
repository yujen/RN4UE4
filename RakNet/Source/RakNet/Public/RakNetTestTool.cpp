// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "RakNetPrivatePCH.h"
#include "RakNetTestTool.h"
#include "LevelEditor.h"
//#include "MultiBox/MultiBoxBuilder.h"

#include "RakPeerInterface.h"
#include "RakNetTypes.h"
#include "GetTime.h"
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include "Gets.h"


#define LOCTEXT_NAMESPACE "RakNetTestTool"



//IMPLEMENT_MODULE(FRakNetTestTool, RakNetTestTool);
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
	enableServer = !enableServer;
	if (enableServer == false)
	{
		RakNet::RakPeerInterface::DestroyInstance(server);
		UE_LOG(LogRakNetTestTool, Log, TEXT("Server stopped."));
		return;
	}


	UE_LOG(LogRakNetTestTool, Log, TEXT("OnRakNetPingServer"));

	server = RakNet::RakPeerInterface::GetInstance();
	int i = server->GetNumberOfAddresses();

	// Holds packets
	//RakNet::Packet* p;
	
	char* charString = TCHAR_TO_UTF8((L"abc123中文字あいうえおdef"));
	//char* charString = TCHAR_TO_UTF8((L""));
	UE_LOG(LogRakNetTestTool, Log, TEXT("Server response string length: %d"), strlen(charString));
	UE_LOG(LogRakNetTestTool, Log, TEXT("Server response string: %s"), *FString(UTF8_TO_TCHAR(charString)));
	//char* charString = TCHAR_TO_ANSI(L"abc123");
	//char enumData[512] = TCHAR_TO_ANSI(L"abc123中文字串あいうえお");
	//char enumData[512] = "abc123";
	server->SetOfflinePingResponse(charString, (const unsigned int)strlen(charString) + 1);

	// The server has to be started to respond to pings.
	RakNet::SocketDescriptor socketDescriptor(60000, 0);
	bool b = server->Startup(2, &socketDescriptor, 1) == RakNet::RAKNET_STARTED;
	server->SetMaximumIncomingConnections(2);
	if (b)
	{
		UE_LOG(LogRakNetTestTool, Log, TEXT("Server started, waiting for connections."));
		/*
		while (enableServer)
		{
			// Get a packet from either the server or the client
			p = server->Receive();
			if (p == 0)
			{
				continue;
			}

			// Check if this is a network message packet
			switch (p->data[0])
			{
			case ID_UNCONNECTED_PONG:
			{
				unsigned int dataLength;
				RakNet::TimeMS time;
				RakNet::BitStream bsIn(p->data, p->length, false);
				bsIn.IgnoreBytes(1);
				bsIn.Read(time);
				dataLength = p->length - sizeof(unsigned char) - sizeof(RakNet::TimeMS);
				printf("ID_UNCONNECTED_PONG from SystemAddress %s.\n", p->systemAddress.ToString(true));
				printf("Time is %i\n", time);
				printf("Ping is %i\n", (unsigned int)(RakNet::GetTimeMS() - time));
				printf("Data is %i bytes long.\n", dataLength);
				if (dataLength > 0)
				{
					printf("Data is %s\n", p->data + sizeof(unsigned char) + sizeof(RakNet::TimeMS));
				}


			}
			break;
			}

			server->DeallocatePacket(p);
		}
		*/

	}

}

void FRakNetTestTool::OnRakNetPingClient()
{
	UE_LOG(LogRakNetTestTool, Log, TEXT("OnRakNetPingClient"));
	RakNet::RakPeerInterface *client = RakNet::RakPeerInterface::GetInstance();

	// Holds packets
	RakNet::Packet* p;


	RakNet::SocketDescriptor socketDescriptor(0, 0);
	client->Startup(1, &socketDescriptor, 1);
	client->Ping("127.0.0.1", 60000, false);

	while (true)
	{
		p = client->Receive();
		if (p == 0)
		{
			continue;
		}

		// Check if this is a network message packet
		switch (p->data[0])
		{
		case ID_UNCONNECTED_PONG:
		{
			unsigned int dataLength;
			RakNet::TimeMS time;
			RakNet::BitStream bsIn(p->data, p->length, false);
			bsIn.IgnoreBytes(1);
			bsIn.Read(time);
			dataLength = p->length - sizeof(unsigned char) - sizeof(RakNet::TimeMS);

			FString sysAddress = FString(p->systemAddress.ToString(true));
			UE_LOG(LogRakNetTestTool, Log, TEXT("ID_UNCONNECTED_PONG from SystemAddress %s."), *sysAddress);
			UE_LOG(LogRakNetTestTool, Log, TEXT("Time is %i"), time);
			UE_LOG(LogRakNetTestTool, Log, TEXT("Data is %i bytes long"), dataLength);


			if (dataLength > 0)
			{
				char* charData = (char*)(p->data + sizeof(unsigned char) + sizeof(RakNet::TimeMS));
				FString strData = FString(UTF8_TO_TCHAR(charData));
				UE_LOG(LogRakNetTestTool, Log, TEXT("String is %s, length is %d "), *strData, strlen(charData));

			}


			/*
			printf("ID_UNCONNECTED_PONG from SystemAddress %s.\n", p->systemAddress.ToString(true));
			printf("Time is %i\n", time);
			printf("Ping is %i\n", (unsigned int)(RakNet::GetTimeMS() - time));
			printf("Data is %i bytes long.\n", dataLength);
			if (dataLength > 0)
				printf("Data is %s\n", p->data + sizeof(unsigned char) + sizeof(RakNet::TimeMS));
			*/


			// In this sample since the client is not running a game we can save CPU cycles by
			// Stopping the network threads after receiving the pong.
			client->Shutdown(100);
		}
		break;
		case ID_UNCONNECTED_PING:
			break;
		case ID_UNCONNECTED_PING_OPEN_CONNECTIONS:
			break;
		}

		client->DeallocatePacket(p);
		break;
	}

	RakNet::RakPeerInterface::DestroyInstance(client);
	UE_LOG(LogRakNetTestTool, Log, TEXT("client finished."));

	/*
	char ip[64];
	puts("Enter IP to ping");
	Gets(ip, sizeof(ip));

	puts(ip);*/


}


#undef LOCTEXT_NAMESPACE
