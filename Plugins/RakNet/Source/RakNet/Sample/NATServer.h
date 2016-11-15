// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RakPeerInterface.h"
#include "NATFramework.h"
#include "RakNetStatistics.h"

#include "GameFramework/Actor.h"
#include "NATServer.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(RakNet_NATServer, Log, All);





UCLASS()
class ANATServer : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANATServer();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	

	UFUNCTION(BlueprintCallable, Category = "RakNet|NATServer")
		void StartServer(const int listeningPort);

	UFUNCTION(BlueprintCallable, Category = "RakNet|NATServer")
		void StopServer();

	UFUNCTION(BlueprintCallable, Category = "RakNet|NATServer")
		void PrintStatistics();

private:

	enum ServerFeatureList
	{
		NAT_TYPE_DETECTION_SERVER,
		NAT_PUNCHTHROUGH_SERVER,
		RELAY_PLUGIN,
		UDP_PROXY_COORDINATOR,
		UDP_PROXY_SERVER,
		CLOUD_SERVER,
		FEATURE_LIST_COUNT
	};

	SampleFramework* sampleFramework[FEATURE_LIST_COUNT];
	RakNet::RakPeerInterface* rakPeer;

};
