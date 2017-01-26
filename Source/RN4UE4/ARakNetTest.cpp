// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "ARakNetTest.h"

DEFINE_LOG_CATEGORY(RakNet_RakNetTest);


ARakNetTest::ARakNetTest()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARakNetTest::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARakNetTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (client == nullptr)
	{
		return;
	}

	p = client->Receive();
	if (p == 0)
	{
		UE_LOG(RakNet_RakNetTest, Log, TEXT("waiting data..."));
		return;
	}

	//waitReceivedData = false;


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
		UE_LOG(RakNet_RakNetTest, Log, TEXT("ID_UNCONNECTED_PONG from SystemAddress %s."), *sysAddress);
		UE_LOG(RakNet_RakNetTest, Log, TEXT("Time is %i"), time);
		UE_LOG(RakNet_RakNetTest, Log, TEXT("Data is %i bytes long"), dataLength);


		if (dataLength > 0)
		{
			char* charData = (char*)(p->data + sizeof(unsigned char) + sizeof(RakNet::TimeMS));
			FString strData = FString(UTF8_TO_TCHAR(charData));
			UE_LOG(RakNet_RakNetTest, Log, TEXT("response string is %s, length is %d "), *strData, strlen(charData));

			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, strData);
			OnReceivePingResponse.Broadcast(strData);

		}

		// In this sample since the client is not running a game we can save CPU cycles by
		// Stopping the network threads after receiving the pong.
		//client->Shutdown(100);
	}
	break;
	case ID_UNCONNECTED_PING:
		break;
	case ID_UNCONNECTED_PING_OPEN_CONNECTIONS:
		break;
	}

	client->DeallocatePacket(p);
	client = nullptr;
	//RakNet::RakPeerInterface::DestroyInstance(server);
	//RakNet::RakPeerInterface::DestroyInstance(client);
	//UE_LOG(RakNet_Ping, Log, TEXT("client receive response!"));

}

void ARakNetTest::ClientPing(const FString& host = "127.0.0.1", const int port = 8888)
{
	UE_LOG(RakNet_RakNetTest, Log, TEXT("APing::ClientPing"));

	client = RakPeerInterface::GetInstance();
	SocketDescriptor socketDescriptor(0, 0);
	client->Startup(1, &socketDescriptor, 1);
	client->Ping(TCHAR_TO_ANSI(*host), port, false);

	//waitReceivedData = true;

}
