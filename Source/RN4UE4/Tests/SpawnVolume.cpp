// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "SpawnVolume.h"
#include "EngineUtils.h"
#include "../RakNetRP.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	active = true;
	objectsSpawned = 0;
}

// Called every frame
void ASpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (rakNetManager != nullptr)
	{
		if (active && rakNetManager->getAllServersChecked() && objectsSpawned < maxObjects)
		{
			currentTime += DeltaTime;
			if (currentTime >= spawnTime)
			{
				chooseRandomPointInBox();
				currentTime = 0.0f;
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

void ASpawnVolume::SetUp()
{

}

ARakNetRP* ASpawnVolume::getRakNetManager()
{
	return rakNetManager;
}

void ASpawnVolume::setRakNetManager(ARakNetRP* raknet)
{
	rakNetManager = raknet;
}

bool ASpawnVolume::getActive()
{
	return active;
}

void ASpawnVolume::setActive(bool act)
{
	active = act;
}

void ASpawnVolume::chooseRandomPointInBox()
{
	FVector pos;
	FVector uniDir;
	pos.X = rakNetManager->getNumberFromRange(GetActorLocation().X - BoxComponent->GetUnscaledBoxExtent().X, GetActorLocation().X + BoxComponent->GetUnscaledBoxExtent().X);
	pos.Y = rakNetManager->getNumberFromRange(GetActorLocation().Z - BoxComponent->GetUnscaledBoxExtent().Z, GetActorLocation().Z + BoxComponent->GetUnscaledBoxExtent().Z);
	pos.Z = rakNetManager->getNumberFromRange(GetActorLocation().Y - BoxComponent->GetUnscaledBoxExtent().Y, GetActorLocation().Y + BoxComponent->GetUnscaledBoxExtent().Y);
	pos = pos / 50.0f;
	uniDir = rakNetManager->getRandomUnitVector();
	rakNetManager->RPrpcSpawn(pos, uniDir);
	++objectsSpawned;
}

void ASpawnVolume::reset()
{
	currentTime = 0.0f;
	objectsSpawned = 0;
}