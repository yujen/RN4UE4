// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RakPeerInterface.h"
#include "NATFramework.h"


#include "GameFramework/Actor.h"
#include "NATClient.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(RakNet_NATClient, Log, All);



// client feature list
UENUM(BlueprintType)
enum ClientFeatureList
{
	_UPNPFramework					UMETA(DisplayName = "UPNPFramework"),
	_NatTypeDetectionFramework		UMETA(DisplayName = "NatTypeDetectionFramework"),
	_NatPunchthoughFramework		UMETA(DisplayName = "NatPunchthoughFramework"),
	_Router2Framework				UMETA(DisplayName = "Router2Framework"),
	_UDPProxyClientFramework		UMETA(DisplayName = "UDPProxyClientFramework"),
	FEATURE_LIST_COUNT
};


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
		void StartClient(const FString& serverAddress, const int serverPort, const int usePort);

	UFUNCTION(BlueprintCallable, Category = "RakNet|NATClient")
		void StopClient();

	UFUNCTION(BlueprintCallable, Category = "RakNet|NATClient")
		void InitFramework(const ClientFeatureList clientFeatureList);


private:

	


	ClientFeatureList currentStage;
	SampleClientFramework* samples[ClientFeatureList::FEATURE_LIST_COUNT];
	RakNet::RakPeerInterface* rakPeer;

	void PrintPacketMessages(RakNet::Packet* packet, RakNet::RakPeerInterface* rakPeer);

};
