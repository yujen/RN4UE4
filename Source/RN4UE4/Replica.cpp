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

bool AReplica::DeserializeConstruction(BitStream *constructionBitstream, Connection_RM3 *sourceConnection)
{
	constructionBitstream->Read(geom);
	UStaticMeshComponent* SphereVisual = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass(), 
		this, TEXT("Geom"));
	SphereVisual->RegisterComponent();
	SphereVisual->AttachTo(GetRootComponent());

	switch (geom)
	{
	case PxGeometryType::eSPHERE:
		SphereVisual->SetStaticMesh(sphereMesh);
		//SphereVisual->SetRelativeLocation(FVector(0.0f, -0.5f, 0.0f));
		break;
	case PxGeometryType::ePLANE:
		break;
	case PxGeometryType::eCAPSULE:
		SphereVisual->SetStaticMesh(capsuleMesh);
		break;
	case PxGeometryType::eBOX:
		SphereVisual->SetStaticMesh(cubeMesh);
		break;
	case PxGeometryType::eCONVEXMESH:
		break;
	case PxGeometryType::eTRIANGLEMESH:
		break;
	case PxGeometryType::eHEIGHTFIELD:
		break;
	case PxGeometryType::eGEOMETRY_COUNT:
		break;
	case PxGeometryType::eINVALID:
		break;
	default:
		break;
	}

	SphereVisual->SetWorldScale3D(FVector(1.0f));

	return SampleReplica::DeserializeConstruction(constructionBitstream, sourceConnection);
}

void AReplica::Deserialize(DeserializeParameters *deserializeParameters) 
{
	SampleReplica::Deserialize(deserializeParameters);
	FQuat rot = FQuat(rotX, rotY, rotZ, rotW);
	FTransform transform = FTransform(FRotator(rot), FVector(posX * 10.0f, posZ * 10.0f, posY * 10.0f), FVector(1,1,1));
	SetActorTransform(transform, false, nullptr, ETeleportType::TeleportPhysics);
}
