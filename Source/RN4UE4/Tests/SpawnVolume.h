// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

class ARakNetRP;

UCLASS()
class RN4UE4_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnVolume();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetUp();
	ARakNetRP*	getRakNetManager();
	void		setRakNetManager(ARakNetRP* raknet);
	bool		getActive();
	void		setActive(bool act);
	void		reset();
	void		chooseRandomPointInBox();
	UPROPERTY(EditAnywhere, Category = "SpawnerBox")
		UBoxComponent* BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	UPROPERTY(EditAnywhere, Category = "Raknet")
		ARakNetRP*		rakNetManager;
	UPROPERTY(EditAnywhere, Category = "Spawner")
		int				maxObjects;
	UPROPERTY(EditAnywhere, Category = "Spawner")
		float			spawnTime;
private:
	bool			active;
	float			currentTime;
	int				objectsSpawned;
	
	
	
};
