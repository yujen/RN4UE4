// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "Replica.h"
#include "PhysXIncludes.h" 

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

void AReplica::OnConstruction(const RigidDynamicConstructionData& data)
{
	FActorSpawnParameters Parameters = FActorSpawnParameters();
	FTransform SpawnTransform = FTransform();
	AStaticMeshActor* shape = nullptr;

	physx::PxGeometryType::Enum geomType = static_cast<physx::PxGeometryType::Enum>(data.geom);

	switch (geomType)
	{
	case physx::PxGeometryType::eSPHERE:
	{
		if (sphereBP == nullptr) break;

		shape = sphereBP->GetDefaultObject<AStaticMeshActor>();
	}
		break;
	case physx::PxGeometryType::ePLANE:
		break;
	case physx::PxGeometryType::eCAPSULE:
	{
		if (capsuleBP == nullptr) break;

		shape = capsuleBP->GetDefaultObject<AStaticMeshActor>();
	}
		break;
	case physx::PxGeometryType::eBOX:
	{
		if (boxBP == nullptr) break;

		shape = boxBP->GetDefaultObject<AStaticMeshActor>();
	}
		break;
	case physx::PxGeometryType::eCONVEXMESH:
		break;
	case physx::PxGeometryType::eTRIANGLEMESH:
		break;
	case physx::PxGeometryType::eHEIGHTFIELD:
		break;
	case physx::PxGeometryType::eGEOMETRY_COUNT:
		break;
	case physx::PxGeometryType::eINVALID:
		break;
	default:
		break;
	}

	if (shape != nullptr)
	{
		Parameters.Template = shape;
		visual = GetWorld()->SpawnActor(shape->GetClass(), &SpawnTransform, Parameters);
	}

	if (visual != nullptr)
	{
		visual->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true));
	}

	pos = data.pos;
	rot = data.rot;

	UpdateTransform();
}

void AReplica::Deserialize(DeserializeParameters *deserializeParameters)
{
	ReplicaRigidDynamic::Deserialize(deserializeParameters);
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

	FQuat	newRot		= FQuat(rot.X, rot.Y, rot.Z, rot.W);
	FVector newPos		= FVector(pos.X, pos.Y, pos.Z) * 50.0f;
	FVector scale		= FVector(-1, 1, 1);					// X will get negated, so set scale to -1, so final result is 1
	FTransform transform = FTransform(FRotator(newRot), newPos, scale);
	transform *= FTransform(conversionMatrix);
	SetActorTransform(transform, false, nullptr, ETeleportType::TeleportPhysics);
}

void AReplica::SetMaterial(int32 elementIndex, UMaterialInterface* inMaterial)
{
	TArray<UStaticMeshComponent*> components;
	if (visual == nullptr)
	{
		UE_LOG(RakNet_Replica, Error, TEXT("Replica::SetMaterial() visual is null, material not set"));
		return;
	}

	visual->GetComponents<UStaticMeshComponent>(components);
	for (int32 i = 0; i < components.Num(); i++)
	{
		UStaticMeshComponent* StaticMeshComponent = components[i];
		StaticMeshComponent->SetMaterial(elementIndex, inMaterial);
	}
}

void AReplica::PostDeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection)
{
	unsigned short port = sourceConnection->GetSystemAddress().GetPort();
	int rank = port - 12345;

	switch (rank)
	{
	case 0:
		SetMaterial(0, server0Material);
		break;
	case 1:
		SetMaterial(0, server1Material);
		break;
	case 2:
		SetMaterial(0, server2Material);
		break;
	case 3:
		SetMaterial(0, server3Material);
		break;
	default:
		SetMaterial(0, unknownMaterial);
		break;
	}
}
