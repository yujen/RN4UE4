// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "RakNetRP.h"

DEFINE_LOG_CATEGORY(RakNet_RakNetRP);

class SampleConnection : public Connection_RM3 
{
public:
	SampleConnection(const SystemAddress &_systemAddress, RakNetGUID _guid) : Connection_RM3(_systemAddress, _guid) {}
	virtual ~SampleConnection() {}

	// See documentation - Makes all messages between ID_REPLICA_MANAGER_DOWNLOAD_STARTED and ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE arrive in one tick
	bool QueryGroupDownloadMessages(void) const { return true; }

	virtual Replica3 *AllocReplica(BitStream *allocationId, ReplicaManager3 *replicaManager3)
	{
		ARakNetRP* manager = (ARakNetRP*)replicaManager3;
		RakString typeName;
		allocationId->Read(typeName);
		return manager->GetObjectFromType(typeName);
	}
};

// Sets default values
ARakNetRP::ARakNetRP()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARakNetRP::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARakNetRP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (rakPeer == nullptr)	{	return;	}

	for (p = rakPeer->Receive(); p; rakPeer->DeallocatePacket(p), p = rakPeer->Receive())
	{
		//UE_LOG(RakNet_RakNetRP, Log, TEXT("PacketRecv:%u"), (unsigned int)p->data[0]);
		switch (p->data[0])
		{
		case ID_CONNECTION_ATTEMPT_FAILED:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_CONNECTION_ATTEMPT_FAILED\n"));
			//quit = true;
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_NO_FREE_INCOMING_CONNECTIONS\n"));
			//quit = true;
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_CONNECTION_REQUEST_ACCEPTED\n"));
			break;
		case ID_NEW_INCOMING_CONNECTION:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_NEW_INCOMING_CONNECTION from %s\n"), p->systemAddress.ToString());
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_DISCONNECTION_NOTIFICATION\n"));
			break;
		case ID_CONNECTION_LOST:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_CONNECTION_LOST\n"));
			break;
		case ID_ADVERTISE_SYSTEM:
			// The first conditional is needed because ID_ADVERTISE_SYSTEM may be from a system we are connected to, but replying on a different address.
			// The second conditional is because AdvertiseSystem also sends to the loopback
			if (rakPeer->GetSystemAddressFromGuid(p->guid) == UNASSIGNED_SYSTEM_ADDRESS &&
				rakPeer->GetMyGUID() != p->guid)
			{
				UE_LOG(RakNet_RakNetRP, Log, TEXT("Connecting to %s\n"), p->systemAddress.ToString(true));
				rakPeer->Connect(p->systemAddress.ToString(false), p->systemAddress.GetPort(), 0, 0);
			}
			break;
		case ID_SND_RECEIPT_LOSS:
		case ID_SND_RECEIPT_ACKED:
		{
			uint32_t msgNumber;
			memcpy(&msgNumber, p->data + 1, 4);

			DataStructures::List<Replica3*> replicaListOut;
			GetReplicasCreatedByMe(replicaListOut);
			unsigned int idx;
			for (idx = 0; idx < replicaListOut.Size(); idx++)
			{
				((SampleReplica*)replicaListOut[idx])->NotifyReplicaOfMessageDeliveryStatus(p->guid, msgNumber, p->data[0] == ID_SND_RECEIPT_ACKED);

				if (idx == 0)
				{
					SampleReplica* replica = (SampleReplica*)replicaListOut[idx];
				}
			}
		}
		break;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		if (rakPeer->GetInternalID(UNASSIGNED_SYSTEM_ADDRESS, 0).GetPort() != SERVER_PORT + i)
			rakPeer->AdvertiseSystem("255.255.255.255", SERVER_PORT + i, 0, 0, 0);
	}
}

void ARakNetRP::RPConnect(const FString& host, const int port)
{
	UE_LOG(RakNet_RakNetRP, Log, TEXT("ARakNetRP::RPConnect"));

	rakPeer = RakPeerInterface::GetInstance();
	SocketDescriptor socketDescriptor(0, 0);
	rakPeer->Startup(1, &socketDescriptor, 1);

	// Start RakNet, up to 32 connections if the server
	rakPeer->AttachPlugin(this);
	SetNetworkIDManager(&networkIdManager);
	rakPeer->SetMaximumIncomingConnections(32);
	rakPeer->Connect(TCHAR_TO_ANSI(*host), port, nullptr, 0);
}

void ARakNetRP::RPDisconnect()
{
	if (rakPeer != nullptr)
	{
		rakPeer->Shutdown(100, 0);
		RakPeerInterface::DestroyInstance(rakPeer);
	}

	delete p;
	p = nullptr;
}

AReplica* ARakNetRP::GetObjectFromType(RakString typeName)
{
	if (typeName == "ServerCreated_ServerSerialized") {
		// spawn the object 
		return GetWorld()->SpawnActor<AReplica>(objectToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, FActorSpawnParameters());
	}

	return nullptr;
}

Connection_RM3* ARakNetRP::AllocConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID) const {
	return new SampleConnection(systemAddress, rakNetGUID);
}

void ARakNetRP::DeallocConnection(Connection_RM3 *connection) const {
	delete connection;
}