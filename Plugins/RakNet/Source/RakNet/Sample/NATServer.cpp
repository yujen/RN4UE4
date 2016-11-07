// Fill out your copyright notice in the Description page of Project Settings.

#include "RakNetPrivatePCH.h"
#include "NATServer.h"
#include "RakPeerInterface.h"


DEFINE_LOG_CATEGORY(RakNet_NATServer);


// Sets default values
ANATServer::ANATServer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

}

void ANATServer::StartServer(const int listeningPort)
{
	RakNet::RakPeerInterface *rakPeer = RakNet::RakPeerInterface::GetInstance();
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

}

