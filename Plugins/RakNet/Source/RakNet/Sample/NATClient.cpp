// Fill out your copyright notice in the Description page of Project Settings.

#include "RakNetPrivatePCH.h"

#include "NATClient.h"


DEFINE_LOG_CATEGORY(RakNet_NATClient);



// Sets default values
ANATClient::ANATClient()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ANATClient::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ANATClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



	samples[(int)currentStage]->Update(rakPeer);
	RakNet::Packet* packet;
	for (packet = rakPeer->Receive(); packet; rakPeer->DeallocatePacket(packet), packet = rakPeer->Receive())
	{
		for (int i = 0; i < FEATURE_LIST_COUNT; i++)
		{
			samples[i]->ProcessPacket(packet);
		}

		PrintPacketMessages(packet, rakPeer);
	}


}

void ANATClient::StartClient(const FString& serverAddress, const int serverPort, const int usePort)
{
	rakPeer = RakNet::RakPeerInterface::GetInstance();
	RakNet::SocketDescriptor sd(usePort, 0);	// #define DEFAULT_RAKPEER_PORT 50000
	if (rakPeer->Startup(32, &sd, 1) != RakNet::RAKNET_STARTED)
	{
		UE_LOG(RakNet_NATClient, Warning, TEXT("Failed to start rakPeer! Quitting."));
		RakNet::RakPeerInterface::DestroyInstance(rakPeer);
		return;
	}
	rakPeer->SetMaximumIncomingConnections(32);


	//SampleClientFramework* samples[ClientFeatureList::FEATURE_LIST_COUNT];
	unsigned int i = 0;
	samples[i++] = new UPNPFramework;
	samples[i++] = new NatTypeDetectionFramework;
	samples[i++] = new NatPunchthoughClientFramework;
	samples[i++] = new Router2Framework;
	samples[i++] = new UDPProxyClientFramework;
	check(i == ClientFeatureList::FEATURE_LIST_COUNT);

}

void ANATClient::StopClient()
{
	rakPeer->Shutdown(0);
	RakNet::RakPeerInterface::DestroyInstance(rakPeer);
	rakPeer = nullptr;
}

void ANATClient::InitFramework(const ClientFeatureList clientFeatureList)
{
	currentStage = clientFeatureList;
	samples[(int)currentStage]->Init(rakPeer);

	PrimaryActorTick.bCanEverTick = true;
}

void ANATClient::PrintPacketMessages(RakNet::Packet* packet, RakNet::RakPeerInterface* rakPeer)
{

	switch (packet->data[0])
	{
	case ID_DISCONNECTION_NOTIFICATION:
		// Connection lost normally
		UE_LOG(RakNet_NATClient, Log, TEXT("ID_DISCONNECTION_NOTIFICATION"));
		break;
	case ID_NEW_INCOMING_CONNECTION:
		UE_LOG(RakNet_NATClient, Log, TEXT("ID_NEW_INCOMING_CONNECTION"));
		break;
	case ID_ALREADY_CONNECTED:
		// Connection lost normally
		UE_LOG(RakNet_NATClient, Log, TEXT("ID_ALREADY_CONNECTED"));
		break;
	case ID_INCOMPATIBLE_PROTOCOL_VERSION:
		UE_LOG(RakNet_NATClient, Log, TEXT("ID_INCOMPATIBLE_PROTOCOL_VERSION"));
		break;
	case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		UE_LOG(RakNet_NATClient, Log, TEXT("ID_REMOTE_DISCONNECTION_NOTIFICATION"));
		break;
	case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		UE_LOG(RakNet_NATClient, Log, TEXT("ID_REMOTE_CONNECTION_LOST"));
		break;
	case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
		UE_LOG(RakNet_NATClient, Log, TEXT("ID_REMOTE_NEW_INCOMING_CONNECTION"));
		break;
	case ID_CONNECTION_BANNED: // Banned from this server
		UE_LOG(RakNet_NATClient, Log, TEXT("We are banned from this server."));
		break;
	case ID_CONNECTION_ATTEMPT_FAILED:
		UE_LOG(RakNet_NATClient, Log, TEXT("Connection attempt failed."));
		break;
	case ID_NO_FREE_INCOMING_CONNECTIONS:
		UE_LOG(RakNet_NATClient, Log, TEXT("ID_NO_FREE_INCOMING_CONNECTIONS"));
		break;
	case ID_INVALID_PASSWORD:
		UE_LOG(RakNet_NATClient, Log, TEXT("ID_INVALID_PASSWORD"));
		break;
	case ID_CONNECTION_LOST:
		UE_LOG(RakNet_NATClient, Log, TEXT("ID_CONNECTION_LOST from %s"), ANSI_TO_TCHAR(packet->systemAddress.ToString(true)));
		break;
	case ID_CONNECTION_REQUEST_ACCEPTED:
		// This tells the client they have connected
		UE_LOG(RakNet_NATClient, Log, TEXT("ID_CONNECTION_REQUEST_ACCEPTED to %s with GUID %s\nMy external address is %s"), ANSI_TO_TCHAR(packet->systemAddress.ToString(true)), ANSI_TO_TCHAR(packet->guid.ToString()), ANSI_TO_TCHAR(rakPeer->GetExternalID(packet->systemAddress).ToString(true)));
		break;
	}
}
