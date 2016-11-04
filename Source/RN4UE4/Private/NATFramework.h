// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NATServer.h"
#include "RakPeerInterface.h"
#include "RakNetTypes.h"
#include "Gets.h"
#include "NatTypeDetectionServer.h"
#include "NatPunchthroughServer.h"
#include "RelayPlugin.h"
#include "UDPProxyServer.h"
#include "UDPProxyCoordinator.h"
#include "RakSleep.h"
#include "MessageIdentifiers.h"
#include "Getche.h"



struct SampleFramework
{
	virtual const char * QueryName(void) = 0;
	virtual const char * QueryRequirements(void) = 0;
	virtual const char * QueryFunction(void) = 0;
	virtual void Init(RakNet::RakPeerInterface *rakPeer) = 0;
	virtual void ProcessPacket(RakNet::RakPeerInterface *rakPeer, RakNet::Packet *packet) = 0;
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer) = 0;

	FeatureSupport isSupported;
};


struct NatTypeDetectionServerFramework : public SampleFramework
{
	NatTypeDetectionServerFramework() { isSupported = ANATServer::NatTypeDetectionServerFramework_Supported; ntds = 0; }
	virtual const char * QueryName(void) override { return "NatTypeDetectionServer"; }
	virtual const char * QueryRequirements(void) override { return "Requires 4 IP addresses"; }
	virtual const char * QueryFunction(void) override { return "Determines router type to filter by connectable systems.\nOne instance needed, multiple instances may exist to spread workload."; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer) override
	{
		if (isSupported == SUPPORTED)
		{
			ntds = new RakNet::NatTypeDetectionServer;
			rakPeer->AttachPlugin(ntds);

			if (rakPeer->GetNumberOfAddresses() < 4)
			{
				printf("Failed. Not enough IP addresses to bind to.\n");
				rakPeer->DetachPlugin(ntds);
				delete ntds;
				ntds = 0;
				isSupported = UNSUPPORTED;
				return;
			}
			ntds->Startup(rakPeer->GetLocalIP(1), rakPeer->GetLocalIP(2), rakPeer->GetLocalIP(3));
		}
	}
	virtual void ProcessPacket(RakNet::RakPeerInterface *rakPeer, RakNet::Packet *packet) override {}
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer) override
	{
		if (ntds)
		{
			rakPeer->DetachPlugin(ntds);
			delete ntds;
		}
		ntds = 0;
	}

	RakNet::NatTypeDetectionServer *ntds;
};

struct NatPunchthroughServerFramework : public SampleFramework, public RakNet::NatPunchthroughServerDebugInterface_Printf
{
	NatPunchthroughServerFramework() { isSupported = ANATServer::NatPunchthroughServerFramework_Supported; nps = 0; }
	virtual const char * QueryName(void) override { return "NatPunchthroughServerFramework"; }
	virtual const char * QueryRequirements(void) override { return "None"; }
	virtual const char * QueryFunction(void) override { return "Coordinates NATPunchthroughClient."; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer) override
	{
		if (isSupported == SUPPORTED)
		{
			nps = new RakNet::NatPunchthroughServer;
			rakPeer->AttachPlugin(nps);
#ifdef VERBOSE_LOGGING
			nps->SetDebugInterface(this);
#endif
		}
	}
	virtual void ProcessPacket(RakNet::RakPeerInterface *rakPeer, RakNet::Packet *packet) override {}
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer) override
	{
		if (nps)
		{
			rakPeer->DetachPlugin(nps);
			delete nps;
		}
		nps = 0;
	}

	RakNet::NatPunchthroughServer *nps;
};

struct RelayPluginFramework : public SampleFramework
{
	RelayPluginFramework() { isSupported = ANATServer::RelayPlugin_Supported; }
	virtual const char * QueryName(void) override { return "RelayPlugin"; }
	virtual const char * QueryRequirements(void) override { return "None."; }
	virtual const char * QueryFunction(void) override { return "Relays messages between named connections."; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer) override
	{
		if (isSupported == SUPPORTED)
		{
			relayPlugin = new RakNet::RelayPlugin;
			rakPeer->AttachPlugin(relayPlugin);
			relayPlugin->SetAcceptAddParticipantRequests(true);
		}
	}
	virtual void ProcessPacket(RakNet::RakPeerInterface *rakPeer, RakNet::Packet *packet) override
	{
	}
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer) override
	{
		if (relayPlugin)
		{
			rakPeer->DetachPlugin(relayPlugin);
			delete relayPlugin;
			relayPlugin = 0;
		}
	}

	RakNet::RelayPlugin *relayPlugin;
};

struct UDPProxyCoordinatorFramework : public SampleFramework
{
	UDPProxyCoordinatorFramework() { udppc = 0; isSupported = ANATServer::UDPProxyCoordinatorFramework_Supported; }
	virtual const char * QueryName(void) override { return "UDPProxyCoordinator"; }
	virtual const char * QueryRequirements(void) override { return "Bandwidth to handle a few hundred bytes per game session."; }
	virtual const char * QueryFunction(void) override { return "Coordinates UDPProxyClient to find available UDPProxyServer.\nExactly one instance required."; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer) override
	{
		if (isSupported == SUPPORTED)
		{
			udppc = new RakNet::UDPProxyCoordinator;
			rakPeer->AttachPlugin(udppc);

			char password[512];
			printf("Create password for UDPProxyCoordinator: ");
			Gets(password, sizeof(password));
			if (password[0] == 0)
			{
				password[0] = 'a';
				password[1] = 0;
			}
			udppc->SetRemoteLoginPassword(password);
		}
	}
	virtual void ProcessPacket(RakNet::RakPeerInterface *rakPeer, RakNet::Packet *packet) override
	{
	}
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer) override
	{
		if (udppc)
		{
			rakPeer->DetachPlugin(udppc);
			delete udppc;
			udppc = 0;
		}
	}

	RakNet::UDPProxyCoordinator *udppc;
};


/*
struct UDPProxyServerFramework : public SampleFramework, public RakNet::UDPProxyServerResultHandler
{
	RakNet::SystemAddress ConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *hostName)
	{
		char ipAddr[64];
		printf("Enter IP of system %s is running on: ", hostName);
		Gets(ipAddr, sizeof(ipAddr));
		if (ipAddr[0] == 0)
		{
			printf("Failed. Not connected to %s.\n", hostName);
			return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		}
		char port[64];
		printf("Enter port of system %s is running on: ", hostName);
		Gets(port, sizeof(port));
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

	RakNet::SystemAddress SelectAmongConnectedSystems(RakNet::RakPeerInterface *rakPeer, const char *hostName)
	{
		DataStructures::List<RakNet::SystemAddress> addresses;
		DataStructures::List<RakNet::RakNetGUID> guids;
		rakPeer->GetSystemList(addresses, guids);
		if (addresses.Size() == 0)
		{
			return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		}
		if (addresses.Size() > 1)
		{
			printf("Select IP address for %s.\n", hostName);
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
	}

	UDPProxyServerFramework() { udpps = 0; isSupported = ANATServer::UDPProxyServerFramework_Supported; }
	virtual const char * QueryName(void) { return "UDPProxyServer"; }
	virtual const char * QueryRequirements(void) { return "Bandwidth to handle forwarded game traffic."; }
	virtual const char * QueryFunction(void) { return "Allows game clients to forward network traffic transparently.\nOne or more instances required, can be added at runtime."; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer)
	{
		if (isSupported == SUPPORTED)
		{
			printf("Logging into UDPProxyCoordinator...\n");
			//RakNet::SystemAddress coordinatorAddress = SelectAmongConnectedSystems(rakPeer, "UDPProxyCoordinator");
			RakNet::SystemAddress coordinatorAddress;
			if (coordinatorAddress == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			{
				printf("Warning: RakPeer is not currently connected to any system.\nEnter option:\n(1). UDPProxyCoordinator is on localhost\n(2). Connect to a remote system\n(3). Fail.\nOption: ");
				char ch = getche();
				printf("\n");
				if (ch == '1' || ch == 13) // 13 is just pressing return
				{
					coordinatorAddress = rakPeer->GetInternalID(RakNet::UNASSIGNED_SYSTEM_ADDRESS, 0);
				}
				else if (ch == '2')
				{
					//coordinatorAddress = ConnectBlocking(rakPeer, "UDPProxyCoordinator");
					if (coordinatorAddress == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
					{
						printf("Failed to connect.\n");
						isSupported = QUERY;
						return;
					}
				}
				else
				{
					printf("Failed. Not connected to UDPProxyCoordinator.\n");
					isSupported = QUERY;
					return;
				}
			}

			char password[512];
			printf("Enter password used with UDPProxyCoordinator: ");
			Gets(password, sizeof(password));
			if (password[0] == 0)
			{
				password[0] = 'a';
				password[1] = 0;
			}

			udpps = new RakNet::UDPProxyServer;
			udpps->SetResultHandler(this);
			rakPeer->AttachPlugin(udpps);
			if (udpps->LoginToCoordinator(password, coordinatorAddress) == false)
			{
				printf("LoginToCoordinator call failed.\n");
				isSupported = QUERY;
				rakPeer->DetachPlugin(udpps);
				delete udpps;
				udpps = 0;
				return;
			}
		}
	}
	virtual void ProcessPacket(RakNet::RakPeerInterface *rakPeer, RakNet::Packet *packet)
	{
	}
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer)
	{
		if (udpps)
		{
			rakPeer->DetachPlugin(udpps);
			delete udpps;
			udpps = 0;
		}
	}

	virtual void OnLoginSuccess(RakNet::RakString usedPassword, RakNet::UDPProxyServer *proxyServerPlugin)
	{
		printf("%s logged into UDPProxyCoordinator.\n", QueryName());
	}
	virtual void OnAlreadyLoggedIn(RakNet::RakString usedPassword, RakNet::UDPProxyServer *proxyServerPlugin)
	{
		printf("%s already logged into UDPProxyCoordinator.\n", QueryName());
	}
	virtual void OnNoPasswordSet(RakNet::RakString usedPassword, RakNet::UDPProxyServer *proxyServerPlugin)
	{
		printf("%s failed login to UDPProxyCoordinator. No password set.\n", QueryName());
		isSupported = QUERY;
		delete udpps;
		udpps = 0;
	}
	virtual void OnWrongPassword(RakNet::RakString usedPassword, RakNet::UDPProxyServer *proxyServerPlugin)
	{
		printf("%s failed login to UDPProxyCoordinator. %s was the wrong password.\n", QueryName(), usedPassword.C_String());
		isSupported = QUERY;
		delete udpps;
		udpps = 0;
	}

	RakNet::UDPProxyServer *udpps;
};
*/
