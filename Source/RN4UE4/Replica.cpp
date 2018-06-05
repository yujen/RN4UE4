// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "Replica.h"
#include "Editor.h"

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
void AReplica::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AReplica::DeserializeConstruction(BitStream *constructionBitstream, Connection_RM3 *sourceConnection)
{
	constructionBitstream->Read(geom);
	FActorSpawnParameters Parameters = FActorSpawnParameters();
	Parameters.bDeferConstruction = true;
	Parameters.bNoFail = true;

	switch (geom)
	{
	case PxGeometryType::eSPHERE:
	{
		if (sphereBP == nullptr) break;

		Parameters.Template = sphereBP;
	}
		break;
	case PxGeometryType::ePLANE:
		break;
	case PxGeometryType::eCAPSULE:
	{
		if (capsuleBP == nullptr) break;

		Parameters.Template = capsuleBP;
	}
		break;
	case PxGeometryType::eBOX:
	{
		if (boxBP == nullptr) break;

		Parameters.Template = boxBP;
	}
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

	//visual = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), GetTransform(), Parameters);
	//const FVector* Location = &GetTransform().GetLocation();
	//const FRotator* Rotator = &GetTransform().GetRotation().Rotator();
	visual = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), GetTransform(), Parameters);
	//visual = GetWorld()->SpawnActor<AStaticMeshActor>(GetTransform().GetLocation(), GetTransform().GetRotation(), NULL, Instigator, true);
	//GEditor->AddActor();

	if (visual != nullptr) visual->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true));

	constructionBitstream->Read(posX);
	constructionBitstream->Read(posY);
	constructionBitstream->Read(posZ);
	constructionBitstream->Read(rotX);
	constructionBitstream->Read(rotY);
	constructionBitstream->Read(rotZ);
	constructionBitstream->Read(rotW);
	UpdateTransform();

	if (replicaManager->GetConnectionAtIndex(0) == sourceConnection)
	{
		SetMaterial(0, server0Material);
	}
	else if (replicaManager->GetConnectionCount() > 1 && replicaManager->GetConnectionAtIndex(1) == sourceConnection)
	{
		SetMaterial(0, server1Material);
	}
	else
	{
		SetMaterial(0, unknownMaterial);
	}

	return SampleReplica::DeserializeConstruction(constructionBitstream, sourceConnection);
}

void AReplica::Deserialize(DeserializeParameters *deserializeParameters)
{
	SampleReplica::Deserialize(deserializeParameters);
	UpdateTransform();
}


bool AReplica::DeserializeDestruction(BitStream *destructionBitstream, Connection_RM3 *sourceConnection)
{
	visual->Destroy();
	return true;
}

void AReplica::UpdateTransform()
{
	// Conversion matrix from PhysX to Unreal
	float	matrixElements[16] = {
		1,  0, 0, 0,
		0,  0, 1, 0,
		0,  1, 0, 0,
		0,  0, 0, 1
	};

	FMatrix conversionMatrix = FMatrix();
	memcpy(conversionMatrix.M, matrixElements, 16 * sizeof(float));

	FQuat	rot		= FQuat(rotX, rotY, rotZ, rotW);
	FVector pos		= FVector(posX, posY, posZ) * 50.0f;
	FVector scale	= FVector(-1, 1, 1);					// X will get negated, so set scale to -1, so final result is 1
	FTransform transform = FTransform(FRotator(rot), pos, scale);
	transform *= FTransform(conversionMatrix);
	SetActorTransform(transform, false, nullptr, ETeleportType::TeleportPhysics);
}

void AReplica::SetMaterial(int32 elementIndex, UMaterialInterface* inMaterial)
{
	TArray<UStaticMeshComponent*> components;
	visual->GetComponents<UStaticMeshComponent>(components);
	for (int32 i = 0; i < components.Num(); i++)
	{
		UStaticMeshComponent* StaticMeshComponent = components[i];
		StaticMeshComponent->SetMaterial(elementIndex, inMaterial);
	}
}
