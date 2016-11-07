// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RakNetTypes.h"
#include "RakPeerInterface.h"
#include "GetTime.h"
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include "Gets.h"

#include "GameFramework/Actor.h"
#include "Ping.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(RakNet_Ping, Log, All);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceivePingResponse, FString, receivedString);


UCLASS()
class APing : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APing();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;


	UFUNCTION(BlueprintCallable, Category = "RakNet|Ping")
		void StartServer(const int listeningPort, const FString& responseString);

	UFUNCTION(BlueprintCallable, Category = "RakNet|Ping")
		void StopServer();

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RakNet|Ping")
	//	FString serverResponseString;

	UFUNCTION(BlueprintCallable, Category = "RakNet|Ping")
		void ClientPing(const FString& host, const int port);



	UPROPERTY(BlueprintAssignable, Category = "RakNet|Ping")
		FOnReceivePingResponse OnReceivePingResponse;


private:

	RakNet::RakPeerInterface* server = nullptr;
	RakNet::RakPeerInterface* client = nullptr;

	// Holds packets
	RakNet::Packet* p = nullptr;

	bool waitReceivedData = false;

};
