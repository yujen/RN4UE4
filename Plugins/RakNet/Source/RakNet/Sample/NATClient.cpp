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

}

void ANATClient::StartConnectServer(const FString& serverAddress, const int serverPort, const int usePort)
{
	RakNet::RakPeerInterface* rakPeer = RakNet::RakPeerInterface::GetInstance();


	RakNet::SocketDescriptor sd(usePort, 0);	// #define DEFAULT_RAKPEER_PORT 50000
	if (rakPeer->Startup(32, &sd, 1) != RakNet::RAKNET_STARTED)
	{
		UE_LOG(RakNet_NATClient, Warning, TEXT("Failed to start rakPeer! Quitting."));
		RakNet::RakPeerInterface::DestroyInstance(rakPeer);
		return;
	}
	rakPeer->SetMaximumIncomingConnections(32);


	SampleClientFramework* samples[ClientFeatureList::FEATURE_LIST_COUNT];
	unsigned int i = 0;
	samples[i++] = new UPNPFramework;
	samples[i++] = new NatTypeDetectionFramework;
	samples[i++] = new NatPunchthoughClientFramework;
	samples[i++] = new Router2Framework;
	samples[i++] = new UDPProxyClientFramework;
	check(i == ClientFeatureList::FEATURE_LIST_COUNT);




	// ====================TODO


	ClientFeatureList currentStage = ClientFeatureList::_UPNPFramework;

	while (samples[(int)currentStage]->QueryRequiresServer() == true)
	{
		UE_LOG(RakNet_NATClient, Log, TEXT("No server: Skipping %s"), ANSI_TO_TCHAR(samples[(int)currentStage]->QueryName()));

		int stageInt = (int)currentStage;
		stageInt++;
		currentStage = (ClientFeatureList)stageInt;
		if (currentStage == FEATURE_LIST_COUNT)
		{
			UE_LOG(RakNet_NATClient, Log, TEXT("Connectivity not possible. Exiting."));
			return;
		}
	}


	while (1)
	{
		printf("Executing %s\n", samples[(int)currentStage]->QueryName());
		samples[(int)currentStage]->Init(rakPeer);

		bool thisSampleDone = false;
		while (1)
		{
			samples[(int)currentStage]->Update(rakPeer);
			RakNet::Packet *packet;
			for (packet = rakPeer->Receive(); packet; rakPeer->DeallocatePacket(packet), packet = rakPeer->Receive())
			{
				for (i = 0; i < FEATURE_LIST_COUNT; i++)
				{
					samples[i]->ProcessPacket(packet);
				}

				PrintPacketMessages(packet, rakPeer);
			}

			if (samples[(int)currentStage]->sampleResult == FAILED ||
				samples[(int)currentStage]->sampleResult == SUCCEEDED)
			{
				printf("\n");
				thisSampleDone = true;
				if (samples[(int)currentStage]->sampleResult == FAILED)
				{
					printf("Failed %s\n", samples[(int)currentStage]->QueryName());

					int stageInt = (int)currentStage;
					stageInt++;
					currentStage = (ClientFeatureList)stageInt;
					if (currentStage == FEATURE_LIST_COUNT)
					{
						printf("Connectivity not possible. Exiting\n");
						rakPeer->Shutdown(100);
						RakNet::RakPeerInterface::DestroyInstance(rakPeer);
						return;
					}
					else
					{
						printf("Proceeding to next stage.\n");
						break;
					}
				}
				else
				{
					printf("Passed %s\n", samples[(int)currentStage]->QueryName());
					if (samples[(int)currentStage]->QueryQuitOnSuccess())
					{

						printf("Press any key to quit.\n");
						while (!kbhit())
						{
							for (packet = rakPeer->Receive(); packet; rakPeer->DeallocatePacket(packet), packet = rakPeer->Receive())
							{
								for (i = 0; i < FEATURE_LIST_COUNT; i++)
								{
									samples[i]->ProcessPacket(packet);
								}

								PrintPacketMessages(packet, rakPeer);
							}
							RakSleep(30);
						}

						rakPeer->Shutdown(100);
						RakNet::RakPeerInterface::DestroyInstance(rakPeer);
						printf("Press enter to quit.\n");
						char temp[32];
						Gets(temp, sizeof(temp));
						return;
					}

					printf("Proceeding to next stage.\n");
					int stageInt = (int)currentStage;
					stageInt++;
					if (stageInt < FEATURE_LIST_COUNT)
					{
						currentStage = (ClientFeatureList)stageInt;
					}
					else
					{
						printf("Press any key to quit when done.\n");

						while (!kbhit())
						{
							for (packet = rakPeer->Receive(); packet; rakPeer->DeallocatePacket(packet), packet = rakPeer->Receive())
							{
								for (i = 0; i < FEATURE_LIST_COUNT; i++)
								{
									samples[i]->ProcessPacket(packet);
								}

								PrintPacketMessages(packet, rakPeer);
							}
							RakSleep(30);
						}

						rakPeer->Shutdown(100);
						RakNet::RakPeerInterface::DestroyInstance(rakPeer);
						return;
					}
					break;
				}
			}

			RakSleep(30);
		}
	}



	//PrimaryActorTick.bCanEverTick = true;
}


void ANATClient::PrintPacketMessages(RakNet::Packet* packet, RakNet::RakPeerInterface* rakPeer)
{

	switch (packet->data[0])
	{
	case ID_DISCONNECTION_NOTIFICATION:
		// Connection lost normally
		printf("ID_DISCONNECTION_NOTIFICATION\n");
		break;
	case ID_NEW_INCOMING_CONNECTION:
		printf("ID_NEW_INCOMING_CONNECTION\n");
		break;
	case ID_ALREADY_CONNECTED:
		// Connection lost normally
		printf("ID_ALREADY_CONNECTED\n");
		break;
	case ID_INCOMPATIBLE_PROTOCOL_VERSION:
		printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
		break;
	case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		printf("ID_REMOTE_DISCONNECTION_NOTIFICATION\n");
		break;
	case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		printf("ID_REMOTE_CONNECTION_LOST\n");
		break;
	case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
		printf("ID_REMOTE_NEW_INCOMING_CONNECTION\n");
		break;
	case ID_CONNECTION_BANNED: // Banned from this server
		printf("We are banned from this server.\n");
		break;
	case ID_CONNECTION_ATTEMPT_FAILED:
		printf("Connection attempt failed\n");
		break;
	case ID_NO_FREE_INCOMING_CONNECTIONS:
		printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
		break;

	case ID_INVALID_PASSWORD:
		printf("ID_INVALID_PASSWORD\n");
		break;

	case ID_CONNECTION_LOST:
		printf("ID_CONNECTION_LOST from %s\n", packet->systemAddress.ToString(true));
		break;

	case ID_CONNECTION_REQUEST_ACCEPTED:
		// This tells the client they have connected
		printf("ID_CONNECTION_REQUEST_ACCEPTED to %s with GUID %s\n", packet->systemAddress.ToString(true), packet->guid.ToString());
		printf("My external address is %s\n", rakPeer->GetExternalID(packet->systemAddress).ToString(true));
		break;
	}
}
