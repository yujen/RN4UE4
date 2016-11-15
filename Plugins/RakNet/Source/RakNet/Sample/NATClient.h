// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RakPeerInterface.h"
#include "NATFramework.h"


#include "GameFramework/Actor.h"
#include "NATClient.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(RakNet_NATClient, Log, All);


UCLASS()
class ANATClient : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANATClient();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;


	UFUNCTION(BlueprintCallable, Category = "RakNet|NATClient")
		void StartConnectServer(const FString& serverAddress, const int serverPort, const int usePort);


private:

	// client feature list
	enum ClientFeatureList
	{
		_UPNPFramework,
		_NatTypeDetectionFramework,
		_NatPunchthoughFramework,
		_Router2Framework,
		_UDPProxyClientFramework,
		FEATURE_LIST_COUNT
	};


	void PrintPacketMessages(RakNet::Packet* packet, RakNet::RakPeerInterface* rakPeer);

};
