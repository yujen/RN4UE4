// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RN4UE4.h"
#include "RakNet.h"
#include "RakNetTypes.h"
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "BitStream.h"
#include "ARakNetTest.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(RakNet_RakNetTest, Log, All);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceivePingResponse, FString, receivedString);

using namespace RakNet;

UCLASS()
class ARakNetTest : public AActor
{
	GENERATED_BODY()
public:
	ARakNetTest();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RakNet|Ping")
	//	FString serverResponseString;

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetTest")
		void ClientPing(const FString& host, const int port);



	UPROPERTY(BlueprintAssignable, Category = "RakNet|RakNetTest")
		FOnReceivePingResponse OnReceivePingResponse;

private:

	RakPeerInterface* server = nullptr;
	RakPeerInterface* client = nullptr;

	// Holds packets
	Packet* p = nullptr;

	bool waitReceivedData = false;
};
