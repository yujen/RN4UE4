// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "BaseReplica.h"
#include "../RakNetRP.h"

// Sets default values
ABaseReplica::ABaseReplica()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	spawnTime = 5.0f;
}

// Called when the game starts or when spawned
void ABaseReplica::BeginPlay()
{
	Super::BeginPlay();
	sent = false;
	active = true;
}

// Called every frame
void ABaseReplica::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (rakNetManager != nullptr)
	{
		if(currentTime <= spawnTime)
			currentTime += DeltaTime;
		if (active && !sent && rakNetManager->getAllServersChecked() && currentTime > spawnTime)
		{
			sendMeshServer();
			sent = true;
			if (MeshComponent != nullptr)
			{
				MeshComponent->DestroyComponent();
			}
		}
	}
	else
	{
		for (TActorIterator<ARakNetRP> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
			if (*ActorItr != nullptr)
			{
				ARakNetRP *rak = *ActorItr;
				rakNetManager = rak;
				break;
			}

		}
	}
}

ARakNetRP* ABaseReplica::getRakNetManager()
{
	return rakNetManager;
}

void ABaseReplica::setRakNetManager(ARakNetRP* raknet)
{
	rakNetManager = raknet;
}

bool ABaseReplica::getActive()
{
	return active;
}

void ABaseReplica::setActive(bool act)
{
	active = act;
}

int ABaseReplica::getMeshType()
{
	return meshType;
}

void ABaseReplica::setMeshType(int mesh)
{
	meshType = mesh;
}

void ABaseReplica::reset()
{
	sent = false;
	currentTime = 0;
}

void ABaseReplica::sendMeshServer()
{
	FVector pos(GetActorLocation().X, GetActorLocation().Z, GetActorLocation().Y);
	pos = pos / 50.0f;
	rakNetManager->RPrpcSpawnType(pos, ArrowComponent->GetForwardVector(),GetActorRotation().Quaternion(),GetActorScale() , meshType);
}