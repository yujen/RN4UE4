// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "Replica.h"

DEFINE_LOG_CATEGORY(RakNet_Replica);


// Sets default values
AReplica::AReplica()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AReplica::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AReplica::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AReplica::Deserialize(DeserializeParameters *deserializeParameters) {
	SampleReplica::Deserialize(deserializeParameters); 
	
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White, TEXT("Deserialised called"));

	SetActorLocation(FVector(var1Unreliable, var2Unreliable, var3Reliable), false, nullptr, ETeleportType::TeleportPhysics);
}
