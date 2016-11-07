// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "NATServer.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(RakNet_NATServer, Log, All);


enum FeatureSupport
{
	SUPPORTED,
	UNSUPPORTED,
	QUERY
};

enum FeatureList
{
	NAT_TYPE_DETECTION_SERVER,
	NAT_PUNCHTHROUGH_SERVER,
	RELAY_PLUGIN,
	UDP_PROXY_COORDINATOR,
	UDP_PROXY_SERVER,
	CLOUD_SERVER,
	FEATURE_LIST_COUNT,
};




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

	static const FeatureSupport NatTypeDetectionServerFramework_Supported = FeatureSupport::QUERY;
	static const FeatureSupport NatPunchthroughServerFramework_Supported = FeatureSupport::QUERY;
	static const FeatureSupport  RelayPlugin_Supported = FeatureSupport::QUERY;
	static const FeatureSupport  UDPProxyCoordinatorFramework_Supported = FeatureSupport::UNSUPPORTED;
	static const FeatureSupport  UDPProxyServerFramework_Supported = FeatureSupport::UNSUPPORTED;
	static const FeatureSupport  CloudServerFramework_Supported = FeatureSupport::QUERY;

	static const int DEFAULT_RAKPEER_PORT = 61111;


	UFUNCTION(BlueprintCallable, Category = "RakNet|NATServer")
		void StartServer(const int listeningPort);


private:

	




};
