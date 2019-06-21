// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseReplica.generated.h"

class ARakNetRP;
UCLASS()
class RN4UE4_API ABaseReplica : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseReplica();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	ARakNetRP*	getRakNetManager();
	void		setRakNetManager(ARakNetRP* raknet);
	bool		getActive();
	void		setActive(bool act);
	int			getMeshType();
	void		setMeshType(int mesh);
	void		reset();
	void		sendMeshServer();
	UPROPERTY(EditAnywhere, Category = "Raknet")
		ARakNetRP*		rakNetManager;
	UPROPERTY(EditAnywhere, Category = "Mesh")
		int				meshType;
	UPROPERTY(EditAnywhere, Category = "Mesh")
		UStaticMeshComponent* MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Body"));
	UPROPERTY(EditAnywhere, Category = "Mesh")
		UArrowComponent * ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Direction"));
	UPROPERTY(EditAnywhere, Category = "Base")
		float			spawnTime;
protected:
	float			currentTime;
	bool			active;
	bool			sent;
};
