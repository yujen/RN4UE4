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

//#include <stdio.h>
//#include "Kbhit.h"
//#include <string.h>
//#include <stdlib.h>
//#include "RakSleep.h"
//#include "Gets.h"

#include "GameFramework/Actor.h"
#include "RPC4Plugin.h"
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
		void RPStartup();

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPDisconnect();

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPConnect(const FString& host, const int port);

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPrpcSpawn(FVector pos, FVector dir);

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPrpcSpawnType(FVector pos, FVector dir, FQuat rot, FVector scale, int meshType);

	UPROPERTY(EditDefaultsOnly, Category = "Object to spawn")
		TSubclassOf<AReplica> objectToSpawn;

	UPROPERTY(EditAnywhere, Category = "Need total reset")
		bool		reset;

	AReplica* GetObjectFromType(RakString typeName);

	void CreateBoundarySlot(RakNet::BitStream * bitStream, Packet * packet);

	void DeleteBoundarySlot(RakNet::BitStream * bitStream, Packet * packet);

	void CleanReplicasSlot(RakNet::BitStream * bitStream, Packet * packet);

	void CheckServerSlot(RakNet::BitStream * bitStream, Packet * packet);

	void TotalNumberServersSlot(RakNet::BitStream * bitStream, Packet * packet);

	void RPCReset();

	void signalCheckServer();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RakNet|RakNetRP")
		void DeleteBoundaryBox(int rank);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RakNet|RakNetRP")
		void CleanReplicas(int rank);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RakNet|RakNetRP")
		void CreateBoundaryBox(int rank, FVector pos, FVector size);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RakNet|RakNetRP")
		void CreateBoundaryPlane(int rank, FVector pos, FVector normal);

	virtual Connection_RM3* AllocConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID) const;
	virtual void DeallocConnection(Connection_RM3 *connection) const;

	bool				 getAllServersChecked();
	FVector				 getRandomUnitVector();
	float				 getNumberFromRange(float min, float max);

private:

	void ConnectToIP(const FString& address);

	RakPeerInterface*		rakPeer				= nullptr;
	NetworkIDManager		networkIdManager;	// ReplicaManager3 requires NetworkIDManager to lookup pointers from numbers.
	Packet*					p					= nullptr;// Holds packets
	
	RPC4 rpc;
	FRandomStream			rand;
	int						totalServers;
	int						activeTest;
	bool					initChecks;
	bool					resetTime;
	int						numberServersChecked;
	bool					allServersChecked;
	static const int		SERVER_PORT = 12345;
};
