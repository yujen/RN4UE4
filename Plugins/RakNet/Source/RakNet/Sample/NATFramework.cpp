// Fill out your copyright notice in the Description page of Project Settings.

#include "RakNetPrivatePCH.h"
#include "NATFramework.h"


DEFINE_LOG_CATEGORY(RakNet_NATFramework);


RakNet::SystemAddress SelectAmongConnectedSystems(RakNet::RakPeerInterface *rakPeer, const char *hostName)
{
	return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	/*
	DataStructures::List<RakNet::SystemAddress> addresses;
	DataStructures::List<RakNet::RakNetGUID> guids;
	rakPeer->GetSystemList(addresses, guids);
	if (addresses.Size() == 0)
	{
		return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	}
	if (addresses.Size() > 1)
	{
		UE_LOG(RakNet_NATFramework, Log, TEXT("Select IP address for %s"), ANSI_TO_TCHAR(hostName));
		char buff[64];
		for (unsigned int i = 0; i < addresses.Size(); i++)
		{
			addresses[i].ToString(true, buff);
			printf("%i. %s\n", i + 1, buff);
		}
		Gets(buff, sizeof(buff));
		if (buff[0] == 0)
		{
			return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		}
		unsigned int idx = atoi(buff);
		if (idx <= 0 || idx > addresses.Size())
		{
			return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		}
		return addresses[idx - 1];
	}
	else
		return addresses[0];
		*/
}

RakNet::SystemAddress ServerConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *hostName)
{
	char ipAddr[64];
	printf("Enter IP of system %s is running on: ", hostName);
	//Gets(ipAddr, sizeof(ipAddr));
	if (ipAddr[0] == 0)
	{
		printf("Failed. Not connected to %s.\n", hostName);
		return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	}
	char port[64];
	printf("Enter port of system %s is running on: ", hostName);
	//Gets(port, sizeof(port));
	if (port[0] == 0)
	{
		printf("Failed. Not connected to %s.\n", hostName);
		return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	}
	if (rakPeer->Connect(ipAddr, atoi(port), 0, 0) != RakNet::CONNECTION_ATTEMPT_STARTED)
	{
		printf("Failed connect call for %s.\n", hostName);
		return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	}
	printf("Connecting...\n");
	RakNet::Packet *packet;
	while (1)
	{
		for (packet = rakPeer->Receive(); packet; rakPeer->DeallocatePacket(packet), packet = rakPeer->Receive())
		{
			if (packet->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
			{
				return packet->systemAddress;
			}
			else
			{
				return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
			}
			RakSleep(100);
		}
	}
}

RakNet::SystemAddress ClientConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *hostName, const char *defaultAddress, const char *defaultPort)
{
	char ipAddr[64];
	if (defaultAddress == 0 || defaultAddress[0] == 0)
	{
		printf("Enter IP of system %s is running on: ", hostName);
	}
	else
	{
		printf("Enter IP of system %s, or press enter for default: ", hostName);
	}
	if (defaultAddress == 0 || defaultAddress[0] == 0)
	{
		UE_LOG(RakNet_NATFramework, Log, TEXT("Failed. No address entered for %s"), ANSI_TO_TCHAR(hostName));
		return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	}
	else
	{
		strcpy(ipAddr, defaultAddress);
	}

	char port[64];
	if (defaultAddress == 0 || defaultAddress[0] == 0)
	{
		printf("Enter port of system %s is running on: ", hostName);
	}
	else
	{
		printf("Enter port of system %s, or press enter for default: ", hostName);
	}
	if (defaultPort == 0 || defaultPort[0] == 0)
	{
		printf("Failed. No port entered for %s.\n", hostName);
		return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	}
	else
	{
		strcpy(port, defaultPort);
	}

	if (rakPeer->Connect(ipAddr, atoi(port), 0, 0) != RakNet::CONNECTION_ATTEMPT_STARTED)
	{
		UE_LOG(RakNet_NATFramework, Log, TEXT("Failed connect call for %s"), ANSI_TO_TCHAR(hostName));
		return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	}
	UE_LOG(RakNet_NATFramework, Log, TEXT("Connecting..."));
	RakNet::Packet *packet;
	while (1)
	{
		for (packet = rakPeer->Receive(); packet; rakPeer->DeallocatePacket(packet), packet = rakPeer->Receive())
		{
			if (packet->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
			{
				return packet->systemAddress;
			}
			else if (packet->data[0] == ID_NO_FREE_INCOMING_CONNECTIONS)
			{
				UE_LOG(RakNet_NATFramework, Log, TEXT("ID_NO_FREE_INCOMING_CONNECTIONS"));
				return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
			}
			else
			{
				return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
			}
			RakSleep(100);
		}
	}
}
