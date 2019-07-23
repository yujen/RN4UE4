#include "RN4UE4.h"
#include "Runtime/Engine/Classes/PhysicsEngine/AggregateGeom.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodySetup.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodyInstance.h"
#include "ThirdParty/PhysX3/PhysX_3.4/Include/PxMaterial.h"
#include "PhysXIncludes.h" 
#include "PhysXPublic.h"
#include "PhysicsPublic.h"
#include "ReplicaRigidDynamicClient.h"

UReplicaRigidDynamicClient::UReplicaRigidDynamicClient()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UReplicaRigidDynamicClient::BeginPlay()
{
	Super::BeginPlay();

	//ensureMsgf(rakNetManager, TEXT("Unexpected null rakNetManager!"));

	registered = false;
}

void UReplicaRigidDynamicClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (rakNetManager != nullptr)
	{
		if (!registered && rakNetManager!=nullptr && rakNetManager->GetInitialised())
		{
			rakNetManager->Reference(this);
			registered = true;
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

RigidDynamicConstructionData UReplicaRigidDynamicClient::GetConstructionData()
{
	FTransform actorTransform = GetOwner()->GetActorTransform();

	// Conversion matrix from PhysX to Unreal
	float	matrixElements[16] = {
		1,  0, 0, 0,
		0,  0, 1, 0,
		0,  1, 0, 0,
		0,  0, 0, 1
	};

	FMatrix conversionMatrix = FMatrix();
	memcpy(conversionMatrix.M, matrixElements, 16 * sizeof(float));

	actorTransform *= FTransform(conversionMatrix.Inverse());
	actorTransform.ScaleTranslation(1 / 50.0f);

	RigidDynamicConstructionData data;
	data.geom = typeMesh;
	FVector position = actorTransform.GetLocation();
	position = position / 50.0f;
	data.pos.X = position.X;
	data.pos.Y = position.Z;
	data.pos.Z = position.Y;
	data.rot.X = actorTransform.GetRotation().X;
	data.rot.Y = actorTransform.GetRotation().Y;
	data.rot.Z = actorTransform.GetRotation().Z;
	data.rot.W = actorTransform.GetRotation().W;
	TArray<UPrimitiveComponent*> comps;
	GetOwner()->GetComponents(comps);
	for (auto Iter = comps.CreateConstIterator(); Iter; ++Iter)
	{
		UStaticMeshComponent* vismesh = Cast<UStaticMeshComponent>(*Iter);
			if (vismesh)
			{
				data.sca.X = GetOwner()->GetActorScale().X;
				data.sca.Y = GetOwner()->GetActorScale().Z;
				data.sca.Z = GetOwner()->GetActorScale().Y;
				data.forwardVector.X = GetOwner()->GetActorForwardVector().X;
				data.forwardVector.Y = GetOwner()->GetActorForwardVector().Y;
				data.forwardVector.Z = GetOwner()->GetActorForwardVector().Z;
				data.mass = vismesh->GetMass();
				data.inertia.X = vismesh->GetInertiaTensor().X;
				data.inertia.Y = vismesh->GetInertiaTensor().Y;
				data.inertia.Z = vismesh->GetInertiaTensor().Z;
				data.angularDamping = vismesh->GetAngularDamping();
				data.linearDamping = vismesh->GetLinearDamping();
				data.gravityEnabled = vismesh->IsGravityEnabled();
				//if box
				if (typeMesh == 3) {
					FRotator rotAux = GetOwner()->GetActorRotation();
					FVector ext;
					FBoxSphereBounds sph;
					FTransform tr;
					GetOwner()->SetActorRotation(FQuat(0, 0, 0, 1));
					sph = vismesh->CalcBounds(tr);
					data.extents.X = sph.BoxExtent.X;
					data.extents.Y = sph.BoxExtent.Z;
					data.extents.Z = sph.BoxExtent.Y;
					GetOwner()->SetActorRotation(rotAux);
				}

				//if sphere
				else if (typeMesh == 0) {
					float radius;
					FBoxSphereBounds sph;
					FTransform tr;
					vismesh->GetBodySetup()->AggGeom.CalcBoxSphereBounds(sph, tr);
					radius = sph.SphereRadius;
					data.extents.X = radius;
					data.extents.Y = 0.0f;
					data.extents.Z = 0.0f;
				}
				//if capsule
				else if (typeMesh == 2) {
					FRotator rotAux = GetOwner()->GetActorRotation();
					FVector ext;
					FBoxSphereBounds sph;
					FTransform tr;
					GetOwner()->SetActorRotation(FQuat(0, 0, 0, 1));
					vismesh->GetBodySetup()->AggGeom.CalcBoxSphereBounds(sph, tr);
					ext = sph.BoxExtent;
					ext = ext * 0.5;
					data.extents.X = ext.X;
					data.extents.Y = ext.Y - ext.X;
					data.extents.Z = 0.0f;
					GetOwner()->SetActorRotation(rotAux);
				}
				//if mesh
				else if (typeMesh == 4) {
					data.numVertex = vismesh->GetBodySetup()->AggGeom.ConvexElems[0].VertexData.Num();
					for (FVector vec : vismesh->GetBodySetup()->AggGeom.ConvexElems[0].VertexData)
					{
						FVector aux = vec / 50.0f;
						Vec3 ver;
						ver.X = aux.X;
						ver.Y = aux.Z;
						ver.Z = aux.Y;
						data.vertexData.push_back(ver);
					}
				}

				FVector centerMass = vismesh->GetCenterOfMass();
				centerMass = centerMass / 50.0f;
				data.centerMass.X = centerMass.X;
				data.centerMass.Y = centerMass.Z;
				data.centerMass.Z = centerMass.Y;
				data.MaxAngularVelocity = vismesh->GetBodyInstance()->MaxAngularVelocity;
				data.typeName = TCHAR_TO_UTF8(*vismesh->GetBodySetup()->GetName());
				data.restitution = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getRestitution();
				PxCombineMode::Enum restituCombineMode = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getRestitutionCombineMode();
				data.dynamicFriction = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getDynamicFriction();
				data.staticFriction = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getStaticFriction();
				PxCombineMode::Enum frictionCombineMode = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFrictionCombineMode();
				data.referenceCount = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getReferenceCount();
				//PxFlags<PxMaterialFlag::Enum, PxU16> flags = vismesh->GetStaticMeshComponent()->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFlags();
				if (restituCombineMode == PxCombineMode::eAVERAGE)
					data.restitutionCombineMode = 0;
				else if (restituCombineMode == PxCombineMode::eMIN)
					data.restitutionCombineMode = 1;
				else if (restituCombineMode == PxCombineMode::eMULTIPLY)
					data.restitutionCombineMode = 2;
				else if (restituCombineMode == PxCombineMode::eMAX)
					data.restitutionCombineMode = 3;
				if (frictionCombineMode == PxCombineMode::eAVERAGE)
					data.frictionCombineModeInt = 0;
				else if (frictionCombineMode == PxCombineMode::eMIN)
					data.frictionCombineModeInt = 1;
				else if (frictionCombineMode == PxCombineMode::eMULTIPLY)
					data.frictionCombineModeInt = 2;
				else if (frictionCombineMode == PxCombineMode::eMAX)
					data.frictionCombineModeInt = 3;
				//	constructionBitstream->Write<PxMaterialFlags>(flags);
			}
	}
	return data;
}

void UReplicaRigidDynamicClient::Deserialize(DeserializeParameters* deserializeParameters)
{
	ReplicaRigidDynamic::Deserialize(deserializeParameters);
	UpdateTransform();
}

void UReplicaRigidDynamicClient::UpdateTransform()
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

	FQuat	newRot = FQuat(rot.X, rot.Y, rot.Z, rot.W);
	FVector newPos = FVector(pos.X, pos.Y, pos.Z) * 50.0f;
	FVector scale = FVector(-1, 1, 1);					// X will get negated, so set scale to -1, so final result is 1
	FTransform transform = FTransform(FRotator(newRot), newPos, scale);
	transform *= FTransform(conversionMatrix);
	GetOwner()->SetActorTransform(transform, false, nullptr, ETeleportType::TeleportPhysics);
}