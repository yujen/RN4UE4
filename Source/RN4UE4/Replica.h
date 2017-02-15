// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RN4UE4.h"
#include "RakNet.h"
#include "RakNetTypes.h"
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "BitStream.h"
#include "ReplicaManager3.h"
#include "NetworkIDManager.h"
#include "VariableDeltaSerializer.h"
#include "GetTime.h"
#include "Rand.h"

#include "GameFramework/Actor.h"
#include "Replica.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(RakNet_Replica, Log, All);

using namespace RakNet;

struct PxGeometryType
{
	enum Enum
	{
		eSPHERE,
		ePLANE,
		eCAPSULE,
		eBOX,
		eCONVEXMESH,
		eTRIANGLEMESH,
		eHEIGHTFIELD,

		eGEOMETRY_COUNT,	//!< internal use only!
		eINVALID = -1		//!< internal use only!
	};
};

class SampleReplica : public Replica3
{
public:
	SampleReplica() { posX = 0; posY = 0; posZ = 0; rotX = 0; rotY = 0; rotZ = 0; rotW = 0; }
	~SampleReplica() {}
	virtual RakString GetName(void) const = 0;
	virtual void WriteAllocationID(Connection_RM3 *destinationConnection, BitStream *allocationIdBitstream) const {
		allocationIdBitstream->Write(GetName());
	}
	void PrintStringInBitstream(BitStream *bs)
	{
		if (bs->GetNumberOfBitsUsed() == 0) return;
		RakString rakString;
		bs->Read(rakString);
	}

	virtual void SerializeConstruction(BitStream *constructionBitstream, Connection_RM3 *destinationConnection)
	{
		// variableDeltaSerializer is a helper class that tracks what variables were sent to what remote system
		// This call adds another remote system to track
		variableDeltaSerializer.AddRemoteSystemVariableHistory(destinationConnection->GetRakNetGUID());

		constructionBitstream->Write(GetName() + RakString(" SerializeConstruction"));
	}
	virtual bool DeserializeConstruction(BitStream *constructionBitstream, Connection_RM3 *sourceConnection) {
		PrintStringInBitstream(constructionBitstream);
		return true;
	}
	virtual void SerializeDestruction(BitStream *destructionBitstream, Connection_RM3 *destinationConnection)
	{
		// variableDeltaSerializer is a helper class that tracks what variables were sent to what remote system
		// This call removes a remote system
		variableDeltaSerializer.RemoveRemoteSystemVariableHistory(destinationConnection->GetRakNetGUID());

		destructionBitstream->Write(GetName() + RakString(" SerializeDestruction"));

	}
	virtual bool DeserializeDestruction(BitStream *destructionBitstream, Connection_RM3 *sourceConnection)
	{
		PrintStringInBitstream(destructionBitstream);
		return true;
	}
	virtual void DeallocReplica(Connection_RM3 *sourceConnection)
	{
		delete this;
	}

	/// Overloaded Replica3 function
	virtual void OnUserReplicaPreSerializeTick(void)
	{
		/// Required by VariableDeltaSerializer::BeginIdenticalSerialize()
		variableDeltaSerializer.OnPreSerializeTick();
	}

	virtual RM3SerializationResult Serialize(SerializeParameters *serializeParameters)
	{
		return RM3SR_DO_NOT_SERIALIZE;
	}

	virtual void Deserialize(DeserializeParameters *deserializeParameters)
	{
		VariableDeltaSerializer::DeserializationContext deserializationContext;

		// Deserialization is written similar to serialization
		// Note that the Serialize() call above uses two different reliability types. This results in two separate Send calls
		// So Deserialize is potentially called twice from a single Serialize
		variableDeltaSerializer.BeginDeserialize(&deserializationContext, &deserializeParameters->serializationBitstream[0]);
		variableDeltaSerializer.DeserializeVariable(&deserializationContext, posX);
		variableDeltaSerializer.DeserializeVariable(&deserializationContext, posY);
		variableDeltaSerializer.DeserializeVariable(&deserializationContext, posZ);
		variableDeltaSerializer.DeserializeVariable(&deserializationContext, rotX);
		variableDeltaSerializer.DeserializeVariable(&deserializationContext, rotY);
		variableDeltaSerializer.DeserializeVariable(&deserializationContext, rotZ);
		variableDeltaSerializer.DeserializeVariable(&deserializationContext, rotW);
		variableDeltaSerializer.EndDeserialize(&deserializationContext);
	}

	virtual void SerializeConstructionRequestAccepted(BitStream *serializationBitstream, Connection_RM3 *requestingConnection) {
		serializationBitstream->Write(GetName() + RakString(" SerializeConstructionRequestAccepted"));
	}
	virtual void DeserializeConstructionRequestAccepted(BitStream *serializationBitstream, Connection_RM3 *acceptingConnection) {
		PrintStringInBitstream(serializationBitstream);
	}
	virtual void SerializeConstructionRequestRejected(BitStream *serializationBitstream, Connection_RM3 *requestingConnection) {
		serializationBitstream->Write(GetName() + RakString(" SerializeConstructionRequestRejected"));
	}
	virtual void DeserializeConstructionRequestRejected(BitStream *serializationBitstream, Connection_RM3 *rejectingConnection) {
		PrintStringInBitstream(serializationBitstream);
	}

	virtual void OnPoppedConnection(Connection_RM3 *droppedConnection)
	{
		// Same as in SerializeDestruction(), no longer track this system
		variableDeltaSerializer.RemoveRemoteSystemVariableHistory(droppedConnection->GetRakNetGUID());
	}
	void NotifyReplicaOfMessageDeliveryStatus(RakNetGUID guid, uint32_t receiptId, bool messageArrived)
	{
		// When using UNRELIABLE_WITH_ACK_RECEIPT, the system tracks which variables were updated with which sends
		// So it is then necessary to inform the system of messages arriving or lost
		// Lost messages will flag each variable sent in that update as dirty, meaning the next Serialize() call will resend them with the current values
		variableDeltaSerializer.OnMessageReceipt(guid, receiptId, messageArrived);
	}

protected:
	// Demonstrate per-variable synchronization
	// We manually test each variable to the last synchronized value and only send those values that change
	float posX, posY, posZ;
	float rotX, rotY, rotZ, rotW;
	PxGeometryType::Enum geom;

	// Class to save and compare the states of variables this Serialize() to the last Serialize()
	// If the value is different, true is written to the bitStream, followed by the value. Otherwise false is written.
	// It also tracks which variables changed which Serialize() call, so if an unreliable message was lost (ID_SND_RECEIPT_LOSS) those variables are flagged 'dirty' and resent
	VariableDeltaSerializer variableDeltaSerializer;
};

UCLASS()
class RN4UE4_API AReplica : public AActor, public SampleReplica
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AReplica();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, Category = "SphereMesh")
	UStaticMesh* sphereMesh;

	UPROPERTY(EditDefaultsOnly, Category = "CubeMesh")
	UStaticMesh* cubeMesh;

	UPROPERTY(EditDefaultsOnly, Category = "CapsuleMesh")
	UStaticMesh* capsuleMesh;

	virtual RakString GetName(void) const { return RakString("ServerCreated_ServerSerialized"); }
	virtual RM3SerializationResult Serialize(SerializeParameters *serializeParameters)
	{
		return RM3SR_DO_NOT_SERIALIZE;
	}
	virtual RM3ConstructionState QueryConstruction(Connection_RM3 *destinationConnection, ReplicaManager3 *replicaManager3) {
		return QueryConstruction_ServerConstruction(destinationConnection, false);
	}
	virtual bool QueryRemoteConstruction(Connection_RM3 *sourceConnection) {
		return QueryRemoteConstruction_ServerConstruction(sourceConnection, false);
	}
	virtual RM3QuerySerializationResult QuerySerialization(Connection_RM3 *destinationConnection) {
		return QuerySerialization_ServerSerializable(destinationConnection, false);
	}
	virtual RM3ActionOnPopConnection QueryActionOnPopConnection(Connection_RM3 *droppedConnection) const {
		return QueryActionOnPopConnection_Server(droppedConnection);
	}
	virtual void DeallocReplica(Connection_RM3 *sourceConnection) {
		Destroy();
	}

	virtual bool DeserializeConstruction(BitStream *constructionBitstream, Connection_RM3 *sourceConnection);
	virtual void Deserialize(DeserializeParameters *deserializeParameters);

	void UpdateTransform();
};
