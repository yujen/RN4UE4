// Fill out your copyright notice in the Description page of Project Settings.

#include "RakNetPrivatePCH.h"
#include "NATServer.h"
//#include "RakPeerInterface.h"
//#include "NATFramework.h"
//#include "RakNetStatistics.h"


DEFINE_LOG_CATEGORY(RakNet_NATServer);


// Sets default values
ANATServer::ANATServer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ANATServer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ANATServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	RakNet::Packet *packet;
	bool quit = false;
	while (!quit)
	{
		for (packet = rakPeer->Receive(); packet; rakPeer->DeallocatePacket(packet), packet = rakPeer->Receive())
		{
			for (int i = 0; i < FEATURE_LIST_COUNT; i++)
			{
				sampleFramework[i]->ProcessPacket(rakPeer, packet);
			}
		}

		if (kbhit())
		{
			char ch = getch();
			if (ch == 'q')
			{
				quit = true;
			}
			else if (ch == ' ')
			{
				RakNet::RakNetStatistics rns;
				char message[2048];
				bool hasStatistics = rakPeer->GetStatistics(0, &rns);
				if (hasStatistics)
				{
					RakNet::StatisticsToString(&rns, message, 2);
					UE_LOG(RakNet_NATServer, Log, TEXT("SYSTEM 0: %s"), ANSI_TO_TCHAR(message));

					memset(&rns, 0, sizeof(RakNet::RakNetStatistics));
					rakPeer->GetStatistics(RakNet::UNASSIGNED_SYSTEM_ADDRESS, &rns);
					StatisticsToString(&rns, message, 2);
					UE_LOG(RakNet_NATServer, Log, TEXT("STAT SUM: %s"), ANSI_TO_TCHAR(message));
				}
				else
				{
					UE_LOG(RakNet_NATServer, Log, TEXT("No system 0"));
				}

				DataStructures::List<RakNet::SystemAddress> addresses;
				DataStructures::List<RakNet::RakNetGUID> guids;
				rakPeer->GetSystemList(addresses, guids);
				UE_LOG(RakNet_NATServer, Log, TEXT("%i systems connected"), addresses.Size());
			}
		}
		RakSleep(30);
	}

	printf("Quitting.\n");
	for (int i = 0; i < FEATURE_LIST_COUNT; i++)
	{
		sampleFramework[i]->Shutdown(rakPeer);
	}
	rakPeer->Shutdown(0);
	RakNet::RakPeerInterface::DestroyInstance(rakPeer);
	rakPeer = nullptr;

}

void ANATServer::StartServer(const int listeningPort)
{
	rakPeer = RakNet::RakPeerInterface::GetInstance();
	RakNet::SystemAddress ipList[MAXIMUM_NUMBER_OF_INTERNAL_IDS];
	unsigned int i;
	for (i = 0; i < MAXIMUM_NUMBER_OF_INTERNAL_IDS; i++)
	{
		ipList[i] = rakPeer->GetLocalIP(i);
		if (ipList[i] != RakNet::UNASSIGNED_SYSTEM_ADDRESS)
		{
			UE_LOG(RakNet_NATServer, Log, TEXT("IP%i: %s"), (i + 1), ANSI_TO_TCHAR(ipList[i].ToString(false)));
		}
		else
		{
			break;
		}
	}

	// If RakPeer is started on 2 IP addresses, NATPunchthroughServer supports port stride detection, improving success rate
	int sdLen = 1;
	RakNet::SocketDescriptor sd[2];

	sd[0].port = listeningPort;
	UE_LOG(RakNet_NATServer, Log, TEXT("Using port %i"), sd[0].port);
	if (i >= 2)
	{
		strcpy(sd[0].hostAddress, ipList[0].ToString(false));
		sd[1].port = listeningPort + 1;
		strcpy(sd[1].hostAddress, ipList[1].ToString(false));
		sdLen = 2;
	}

	if (rakPeer->Startup(8096, sd, sdLen) != RakNet::RAKNET_STARTED)
	{
		UE_LOG(RakNet_NATServer, Log, TEXT("Failed to start rakPeer! Quitting"));
		RakNet::RakPeerInterface::DestroyInstance(rakPeer);
		return;
	}
	rakPeer->SetTimeoutTime(5000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	UE_LOG(RakNet_NATServer, Log, TEXT("Started on %s"), ANSI_TO_TCHAR(rakPeer->GetMyBoundAddress().ToString(true)));

	rakPeer->SetMaximumIncomingConnections(8096);



	i = 0;
	sampleFramework[i++] = new NatTypeDetectionServerFramework;
	sampleFramework[i++] = new NatPunchthroughServerFramework;
	sampleFramework[i++] = new RelayPluginFramework;
	sampleFramework[i++] = new UDPProxyCoordinatorFramework;
	sampleFramework[i++] = new UDPProxyServerFramework;
	sampleFramework[i++] = new CloudServerFramework;
	check(i == FEATURE_LIST_COUNT);

	// start QUERY feature
	for (i = 0; i < FEATURE_LIST_COUNT; i++)
	{
		if (sampleFramework[i]->isSupported == QUERY)
		{
			sampleFramework[i]->isSupported = SUPPORTED;

			UE_LOG(RakNet_NATServer, Log, TEXT("QUERY=%s, Requirements=%s, Description=%s"),
				ANSI_TO_TCHAR(sampleFramework[i]->QueryName()),
				ANSI_TO_TCHAR(sampleFramework[i]->QueryRequirements()),
				ANSI_TO_TCHAR(sampleFramework[i]->QueryFunction())
			);
		}
	}
	for (i = 0; i < FEATURE_LIST_COUNT; i++)
	{
		if (sampleFramework[i]->isSupported == SUPPORTED)
		{
			sampleFramework[i]->Init(rakPeer);
			if (sampleFramework[i]->isSupported == SUPPORTED)
			{
				UE_LOG(RakNet_NATServer, Log, TEXT("Success starting %s"), ANSI_TO_TCHAR(sampleFramework[i]->QueryName()));
			}
			else
			{
				UE_LOG(RakNet_NATServer, Log, TEXT("Failed to start %s"), ANSI_TO_TCHAR(sampleFramework[i]->QueryName()));
			}
		}
	}

	bool anySupported = false;
	for (i = 0; i < FEATURE_LIST_COUNT; i++)
	{
		if (sampleFramework[i]->isSupported == SUPPORTED)
		{
			anySupported = true;
			break;
		}
	}
	if (anySupported == false)
	{
		UE_LOG(RakNet_NATServer, Warning, TEXT("No features supported! Quitting."));
		rakPeer->Shutdown(0);
		RakNet::RakPeerInterface::DestroyInstance(rakPeer);
		return;
	}

	for (i = 0; i < FEATURE_LIST_COUNT; i++)
	{
		if (sampleFramework[i]->isSupported == SUPPORTED)
		{
			UE_LOG(RakNet_NATServer, Log, TEXT("Supported feature: %s"), ANSI_TO_TCHAR(sampleFramework[i]->QueryName()));
		}
	}

	// start server tick loop
	PrimaryActorTick.bCanEverTick = true;

}

