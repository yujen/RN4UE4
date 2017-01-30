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

#include "GameFramework/Actor.h"
#include "RakNetRP.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(RakNet_RakNetRP, Log, All);


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceiveResponse, FString, receivedString);

// ReplicaManager3 is in the namespace RakNet
using namespace RakNet;

class ReplicaManager3Sample;

UCLASS()
class RN4UE4_API ARakNetRP : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARakNetRP();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP ")
		void RPConnect(const FString& host, const int port);

	//UPROPERTY(BlueprintAssignable, Category = "RakNet|RakNetRP")
	//	FOnReceiveResponse OnReceiveResponse;
	
private:

	RakPeerInterface* rakPeer = nullptr;
	ReplicaManager3Sample* replicaManager = nullptr;
	// ReplicaManager3 requires NetworkIDManager to lookup pointers from numbers.
	NetworkIDManager networkIdManager;

	// Holds packets
	Packet* p = nullptr;


	static const int SERVER_PORT = 12345;
};
