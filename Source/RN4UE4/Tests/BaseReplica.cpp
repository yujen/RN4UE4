// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "BaseReplica.h"
#include "../RakNetRP.h"

// Sets default values
ABaseReplica::ABaseReplica()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseReplica::BeginPlay()
{
	Super::BeginPlay();
	sent = false;
}

// Called every frame
void ABaseReplica::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (rakNetManager != nullptr)
	{
		if (active && !sent && rakNetManager->getAllServersChecked())
		{
			sendMeshServer();
			sent = true;
			MeshComponent->DestroyComponent();
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

int ABaseReplica::reset()
{
	sent = false;
}

void ABaseReplica::sendMeshServer()
{
	rakNetManager->RPrpcSpawnType(GetActorLocation(), GetActorScale(),GetActorRotation().Quaternion(), ArrowComponent->GetForwardVector(), meshType);
}