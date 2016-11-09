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

	

	static const int DEFAULT_RAKPEER_PORT = 61111;


	UFUNCTION(BlueprintCallable, Category = "RakNet|NATServer")
		void StartServer(const int listeningPort);


private:

	SampleFramework* sampleFramework[FEATURE_LIST_COUNT];
	RakNet::RakPeerInterface* rakPeer;

};
