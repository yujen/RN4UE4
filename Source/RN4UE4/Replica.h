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

class SampleReplica : public Replica3
{
public:
	SampleReplica() { var1Unreliable = 0; var2Unreliable = 0; var3Reliable = 0; var4Reliable = 0; }
	~SampleReplica() {}
	virtual RakString GetName(void) const = 0;
	virtual void WriteAllocationID(Connection_RM3 *destinationConnection, BitStream *allocationIdBitstream) const {
		allocationIdBitstream->Write(GetName());
	}
	void PrintStringInBitstream(BitStream *bs)
	{
		if (bs->GetNumberOfBitsUsed() == 0)
			return;
		RakString rakString;
		bs->Read(rakString);
		UE_LOG(RakNet_Replica, Log, TEXT("Receive: %s\n"), rakString.C_String());
	}

	virtual void SerializeConstruction(BitStream *constructionBitstream, Connection_RM3 *destinationConnection) {

		// variableDeltaSerializer is a helper class that tracks what variables were sent to what remote system
		// This call adds another remote system to track
		variableDeltaSerializer.AddRemoteSystemVariableHistory(destinationConnection->GetRakNetGUID());

		constructionBitstream->Write(GetName() + RakString(" SerializeConstruction"));
	}
	virtual bool DeserializeConstruction(BitStream *constructionBitstream, Connection_RM3 *sourceConnection) {
		PrintStringInBitstream(constructionBitstream);
		return true;
	}
	virtual void SerializeDestruction(BitStream *destructionBitstream, Connection_RM3 *destinationConnection) {

		// variableDeltaSerializer is a helper class that tracks what variables were sent to what remote system
		// This call removes a remote system
		variableDeltaSerializer.RemoveRemoteSystemVariableHistory(destinationConnection->GetRakNetGUID());

		destructionBitstream->Write(GetName() + RakString(" SerializeDestruction"));

	}
	virtual bool DeserializeDestruction(BitStream *destructionBitstream, Connection_RM3 *sourceConnection) {
		PrintStringInBitstream(destructionBitstream);
		return true;
	}
	virtual void DeallocReplica(Connection_RM3 *sourceConnection) {
		delete this;
	}

	/// Overloaded Replica3 function
	virtual void OnUserReplicaPreSerializeTick(void)
	{
		/// Required by VariableDeltaSerializer::BeginIdenticalSerialize()
		variableDeltaSerializer.OnPreSerializeTick();
	}

	virtual RM3SerializationResult Serialize(SerializeParameters *serializeParameters) {

		VariableDeltaSerializer::SerializationContext serializationContext;

		// Put all variables to be sent unreliably on the same channel, then specify the send type for that channel
		serializeParameters->pro[0].reliability = UNRELIABLE_WITH_ACK_RECEIPT;
		// Sending unreliably with an ack receipt requires the receipt number, and that you inform the system of ID_SND_RECEIPT_ACKED and ID_SND_RECEIPT_LOSS
		serializeParameters->pro[0].sendReceipt = replicaManager->GetRakPeerInterface()->IncrementNextSendReceipt();
		serializeParameters->messageTimestamp = GetTime();

		// Begin writing all variables to be sent UNRELIABLE_WITH_ACK_RECEIPT 
		variableDeltaSerializer.BeginUnreliableAckedSerialize(
			&serializationContext,
			serializeParameters->destinationConnection->GetRakNetGUID(),
			&serializeParameters->outputBitstream[0],
			serializeParameters->pro[0].sendReceipt
		);
		// Write each variable
		variableDeltaSerializer.SerializeVariable(&serializationContext, var1Unreliable);
		// Write each variable
		variableDeltaSerializer.SerializeVariable(&serializationContext, var2Unreliable);
		// Tell the system this is the last variable to be written
		variableDeltaSerializer.EndSerialize(&serializationContext);

		// All variables to be sent using a different mode go on different channels
		serializeParameters->pro[1].reliability = RELIABLE_ORDERED;

		// Same as above, all variables to be sent with a particular reliability are sent in a batch
		// We use BeginIdenticalSerialize instead of BeginSerialize because the reliable variables have the same values sent to all systems. This is memory-saving optimization
		variableDeltaSerializer.BeginIdenticalSerialize(
			&serializationContext,
			serializeParameters->whenLastSerialized == 0,
			&serializeParameters->outputBitstream[1]
		);
		variableDeltaSerializer.SerializeVariable(&serializationContext, var3Reliable);
		variableDeltaSerializer.SerializeVariable(&serializationContext, var4Reliable);
		variableDeltaSerializer.EndSerialize(&serializationContext);

		// This return type makes is to ReplicaManager3 itself does not do a memory compare. we entirely control serialization ourselves here.
		// Use RM3SR_SERIALIZED_ALWAYS instead of RM3SR_SERIALIZED_ALWAYS_IDENTICALLY to support sending different data to different system, which is needed when using unreliable and dirty variable resends
		return RM3SR_SERIALIZED_ALWAYS;
	}
	virtual void Deserialize(DeserializeParameters *deserializeParameters) {

		VariableDeltaSerializer::DeserializationContext deserializationContext;

		// Deserialization is written similar to serialization
		// Note that the Serialize() call above uses two different reliability types. This results in two separate Send calls
		// So Deserialize is potentially called twice from a single Serialize
		variableDeltaSerializer.BeginDeserialize(&deserializationContext, &deserializeParameters->serializationBitstream[0]);
		if (variableDeltaSerializer.DeserializeVariable(&deserializationContext, var1Unreliable))
			UE_LOG(RakNet_Replica, Log, TEXT("var1Unreliable changed to %f\n"), var1Unreliable);
			if (variableDeltaSerializer.DeserializeVariable(&deserializationContext, var2Unreliable))
				UE_LOG(RakNet_Replica, Log, TEXT("var2Unreliable changed to %f\n"), var2Unreliable);
				variableDeltaSerializer.EndDeserialize(&deserializationContext);

		variableDeltaSerializer.BeginDeserialize(&deserializationContext, &deserializeParameters->serializationBitstream[1]);
		if (variableDeltaSerializer.DeserializeVariable(&deserializationContext, var3Reliable))
			UE_LOG(RakNet_Replica, Log, TEXT("var3Reliable changed to %f\n"), var3Reliable);
			if (variableDeltaSerializer.DeserializeVariable(&deserializationContext, var4Reliable))
				UE_LOG(RakNet_Replica, Log, TEXT("var4Reliable changed to %f\n"), var4Reliable);
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

	// Demonstrate per-variable synchronization
	// We manually test each variable to the last synchronized value and only send those values that change
	float var1Unreliable, var2Unreliable, var3Reliable, var4Reliable;

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

	virtual void Deserialize(DeserializeParameters *deserializeParameters);
	virtual void DeallocReplica(Connection_RM3 *sourceConnection) {
		Destroy();
	}
};
