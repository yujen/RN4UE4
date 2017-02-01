// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "RN4UE4.h"
#include "RakNet.h"
#include "RakNetTypes.h"
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "BitStream.h"
#include "ReplicaManager3.h"
#include "NetworkIDManager.h"
#include "VariableDeltaSerializer.h"
#include "GetTime.h"
#include "Rand.h"

#include "Replica.h"

#include "GameFramework/Actor.h"
#include "RakNetRP.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllocReplica, FString, receivedString);


DECLARE_LOG_CATEGORY_EXTERN(RakNet_RakNetRP, Log, All);

// ReplicaManager3 is in the namespace RakNet
using namespace RakNet;

class ReplicaManager3Sample;

UCLASS()
class RN4UE4_API ARakNetRP : public AActor, public ReplicaManager3
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARakNetRP();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPConnect(const FString& host, const int port);

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPDisconnect();

	UPROPERTY(EditDefaultsOnly, Category = "Object to spawn")
		TSubclassOf<AReplica> objectToSpawn;

	AReplica* GetObjectFromType(RakString typeName);

	virtual Connection_RM3* AllocConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID) const;
	virtual void DeallocConnection(Connection_RM3 *connection) const;

private:

	RakPeerInterface*		rakPeer				= nullptr;
	NetworkIDManager		networkIdManager;	// ReplicaManager3 requires NetworkIDManager to lookup pointers from numbers.
	Packet*					p					= nullptr;// Holds packets


	static const int SERVER_PORT = 12345;
};
