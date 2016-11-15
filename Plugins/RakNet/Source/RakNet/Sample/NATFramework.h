// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
#include "CloudServer.h"
#include "CloudClient.h"
#include "CloudServerHelper.h"
#include "NatTypeDetectionClient.h"
#include "GetTime.h"
#include "NatPunchthroughClient.h"
#include "Router2.h"
#include "UDPProxyClient.h"
#include "Itoa.h"


enum FeatureSupport
{
	SUPPORTED,
	UNSUPPORTED,
	QUERY
};




static const FeatureSupport NatTypeDetectionServerFramework_Supported = FeatureSupport::QUERY;
static const FeatureSupport NatPunchthroughServerFramework_Supported = FeatureSupport::QUERY;
static const FeatureSupport RelayPlugin_Supported = FeatureSupport::QUERY;
static const FeatureSupport UDPProxyCoordinatorFramework_Supported = FeatureSupport::UNSUPPORTED;
static const FeatureSupport UDPProxyServerFramework_Supported = FeatureSupport::UNSUPPORTED;
static const FeatureSupport CloudServerFramework_Supported = FeatureSupport::QUERY;


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

RakNet::SystemAddress SelectAmongConnectedSystems(RakNet::RakPeerInterface *rakPeer, const char *hostName);
RakNet::SystemAddress ServerConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *hostName);
RakNet::SystemAddress ClientConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char *hostName, const char *defaultAddress, const char *defaultPort);



struct NatTypeDetectionServerFramework : public SampleFramework
{
	NatTypeDetectionServerFramework() { isSupported = NatTypeDetectionServerFramework_Supported; ntds = 0; }
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
	NatPunchthroughServerFramework() { isSupported = NatPunchthroughServerFramework_Supported; nps = 0; }
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
	RelayPluginFramework() { isSupported = RelayPlugin_Supported; }
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
	UDPProxyCoordinatorFramework() { udppc = 0; isSupported = UDPProxyCoordinatorFramework_Supported; }
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



struct UDPProxyServerFramework : public SampleFramework, public RakNet::UDPProxyServerResultHandler
{
	UDPProxyServerFramework() { udpps = 0; isSupported = UDPProxyServerFramework_Supported; }
	virtual const char * QueryName(void) { return "UDPProxyServer"; }
	virtual const char * QueryRequirements(void) { return "Bandwidth to handle forwarded game traffic."; }
	virtual const char * QueryFunction(void) { return "Allows game clients to forward network traffic transparently.\nOne or more instances required, can be added at runtime."; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer)
	{
		if (isSupported == SUPPORTED)
		{
			printf("Logging into UDPProxyCoordinator...\n");
			RakNet::SystemAddress coordinatorAddress = SelectAmongConnectedSystems(rakPeer, "UDPProxyCoordinator");
			//RakNet::SystemAddress coordinatorAddress;
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
					coordinatorAddress = ServerConnectBlocking(rakPeer, "UDPProxyCoordinator");
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

struct CloudServerFramework : public SampleFramework
{
	CloudServerFramework() { isSupported = CloudServerFramework_Supported; }
	virtual const char * QueryName(void) { return "CloudServer"; }
	virtual const char * QueryRequirements(void) { return "None."; }
	virtual const char * QueryFunction(void) { return "Single instance cloud server that maintains connection counts\nUseful as a directory server to find other client instances."; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer)
	{
		if (isSupported == SUPPORTED)
		{
			cloudServer = new RakNet::CloudServer;
			rakPeer->AttachPlugin(cloudServer);
			cloudClient = new RakNet::CloudClient;
			rakPeer->AttachPlugin(cloudClient);
			cloudServerHelperFilter = new RakNet::CloudServerHelperFilter;
			cloudServer->AddQueryFilter(cloudServerHelperFilter);
			cloudServer->SetMaxUploadBytesPerClient(65535);
			cloudServerHelper.OnConnectionCountChange(rakPeer, cloudClient);
		}
	}
	virtual void ProcessPacket(RakNet::RakPeerInterface *rakPeer, RakNet::Packet *packet)
	{
		if (isSupported != SUPPORTED)
			return;

		switch (packet->data[0])
		{
		case ID_NEW_INCOMING_CONNECTION:
#ifdef VERBOSE_LOGGING
			printf("Got connection to %s\n", packet->systemAddress.ToString(true));
#endif
			cloudServerHelper.OnConnectionCountChange(rakPeer, cloudClient);
			break;
		case ID_CONNECTION_LOST:
		case ID_DISCONNECTION_NOTIFICATION:
#ifdef VERBOSE_LOGGING
			printf("Lost connection to %s\n", packet->systemAddress.ToString(true));
#endif
			cloudServerHelper.OnConnectionCountChange(rakPeer, cloudClient);
			break;
		}
	}
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer)
	{
		if (cloudServer)
		{
			rakPeer->DetachPlugin(cloudServer);
			delete cloudServer;
			cloudServer = 0;
			rakPeer->DetachPlugin(cloudClient);
			delete cloudClient;
			cloudClient = 0;
			delete cloudServerHelperFilter;
			cloudServerHelperFilter = 0;
		}
	}

	RakNet::CloudServer *cloudServer;
	RakNet::CloudClient *cloudClient;
	RakNet::CloudServerHelperFilter *cloudServerHelperFilter;
	RakNet::CloudServerHelper cloudServerHelper;
};



#define DEFAULT_SERVER_ADDRESS "localhost"
#define DEFAULT_SERVER_PORT "61111"
#define RAKPEER_PORT_STR "0"

#define SUPPORT_UPNP SampleResult::FAILED
#define SUPPORT_NAT_TYPE_DETECTION SampleResult::FAILED
#define SUPPORT_NAT_PUNCHTHROUGH SampleResult::PENDING
#define SUPPORT_ROUTER2 SampleResult::FAILED
#define SUPPORT_UDP_PROXY SampleResult::FAILED


enum SampleResult
{
	PENDING,
	FAILED,
	SUCCEEDED
};

struct SampleClientFramework
{
	virtual const char * QueryName(void) = 0;
	virtual bool QueryRequiresServer(void) = 0;
	virtual const char * QueryFunction(void) = 0;
	virtual const char * QuerySuccess(void) = 0;
	virtual bool QueryQuitOnSuccess(void) = 0;
	virtual void Init(RakNet::RakPeerInterface *rakPeer) = 0;
	virtual void ProcessPacket(RakNet::Packet *packet) = 0;
	virtual void Update(RakNet::RakPeerInterface *rakPeer) = 0;
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer) = 0;

	SampleResult sampleResult;
};

struct UPNPFramework : public SampleClientFramework
{
	UPNPFramework() { sampleResult = SUPPORT_UPNP; }
	virtual const char * QueryName(void) { return "UPNPFramework"; }
	virtual bool QueryRequiresServer(void) { return false; }
	virtual const char * QueryFunction(void) { return "Use UPNP to open the router"; }
	virtual const char * QuerySuccess(void) { return "Other systems can now connect to you on the opened port."; }
	virtual bool QueryQuitOnSuccess(void) { return true; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer)
	{
		// UPNP isn't implemented in UE4
		sampleResult = FAILED;
		return;

		/*
		if (sampleResult == FAILED) return;

		struct UPNPDev * devlist = 0;
		devlist = upnpDiscover(2000, 0, 0, 0, 0, 0);
		if (devlist)
		{
			printf("List of UPNP devices found on the network :\n");
			struct UPNPDev * device;
			for (device = devlist; device; device = device->pNext)
			{
				printf(" desc: %s\n st: %s\n\n",
					device->descURL, device->st);
			}

			char lanaddr[64];	// my ip address on the LAN
			struct UPNPUrls urls;
			struct IGDdatas data;
			if (UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr)) == 1)
			{
				// Use same external and internal ports
				DataStructures::List<RakNet::RakNetSocket2* > sockets;
				rakPeer->GetSockets(sockets);
				char iport[32];
				Itoa(sockets[0]->GetBoundAddress().GetPort(), iport, 10);
				char eport[32];
				strcpy(eport, iport);


				// Version 1.5
				// 				int r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
				// 					 eport, iport, lanaddr, 0, "UDP", 0);

				// Version miniupnpc-1.6.20120410
				int r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
					eport, iport, lanaddr, 0, "UDP", 0, "0");

				if (r != UPNPCOMMAND_SUCCESS)
					printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n",
						eport, iport, lanaddr, r, strupnperror(r));

				char intPort[6];
				char intClient[16];

				// Version 1.5
				// 				r = UPNP_GetSpecificPortMappingEntry(urls.controlURL,
				// 					data.first.servicetype,
				// 					eport, "UDP",
				// 					intClient, intPort);

				// Version miniupnpc-1.6.20120410
				char desc[128];
				char enabled[128];
				char leaseDuration[128];
				r = UPNP_GetSpecificPortMappingEntry(urls.controlURL,
					data.first.servicetype,
					eport, "UDP",
					intClient, intPort,
					desc, enabled, leaseDuration);

				if (r != UPNPCOMMAND_SUCCESS)
				{
					printf("GetSpecificPortMappingEntry() failed with code %d (%s)\n",
						r, strupnperror(r));
					sampleResult = FAILED;
				}
				else
					sampleResult = SUCCEEDED;
			}
			else
				sampleResult = FAILED;
		}
		else
			sampleResult = FAILED;

		*/
	}

	virtual void ProcessPacket(RakNet::Packet *packet)
	{
	}
	virtual void Update(RakNet::RakPeerInterface *rakPeer)
	{
		if (sampleResult == FAILED) return;
	}
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer)
	{
	}

};

struct NatTypeDetectionFramework : public SampleClientFramework
{
	// Set to FAILED to skip this test
	NatTypeDetectionFramework() { sampleResult = SUPPORT_NAT_TYPE_DETECTION; ntdc = 0; }
	virtual const char * QueryName(void) { return "NatTypeDetectionFramework"; }
	virtual bool QueryRequiresServer(void) { return true; }
	virtual const char * QueryFunction(void) { return "Determines router type to avoid NAT punch attempts that cannot\nsucceed."; }
	virtual const char * QuerySuccess(void) { return "If our NAT type is Symmetric, we can skip NAT punch to other symmetric NATs."; }
	virtual bool QueryQuitOnSuccess(void) { return false; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer)
	{
		if (sampleResult == FAILED) return;

		RakNet::SystemAddress serverAddress = SelectAmongConnectedSystems(rakPeer, "NatTypeDetectionServer");
		if (serverAddress == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
		{
			serverAddress = ClientConnectBlocking(rakPeer, "NatTypeDetectionServer", DEFAULT_SERVER_ADDRESS, DEFAULT_SERVER_PORT);
			if (serverAddress == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			{
				printf("Failed to connect to a server.\n");
				sampleResult = FAILED;
				return;
			}
		}
		ntdc = new RakNet::NatTypeDetectionClient;
		rakPeer->AttachPlugin(ntdc);
		ntdc->DetectNATType(serverAddress);
		timeout = RakNet::GetTimeMS() + 5000;
	}

	virtual void ProcessPacket(RakNet::Packet *packet)
	{
		if (packet->data[0] == ID_NAT_TYPE_DETECTION_RESULT)
		{
			RakNet::NATTypeDetectionResult r = (RakNet::NATTypeDetectionResult) packet->data[1];
			printf("NAT Type is %s (%s)\n", NATTypeDetectionResultToString(r), NATTypeDetectionResultToStringFriendly(r));
			printf("Using NATPunchthrough can connect to systems using:\n");
			for (int i = 0; i < (int)RakNet::NAT_TYPE_COUNT; i++)
			{
				if (CanConnect(r, (RakNet::NATTypeDetectionResult)i))
				{
					if (i != 0)
						printf(", ");
					printf("%s", NATTypeDetectionResultToString((RakNet::NATTypeDetectionResult)i));
				}
			}
			printf("\n");
			if (r == RakNet::NAT_TYPE_PORT_RESTRICTED || r == RakNet::NAT_TYPE_SYMMETRIC)
			{
				// For UPNP, see Samples\UDPProxy
				printf("Note: Your router must support UPNP or have the user manually forward ports.\n");
				printf("Otherwise NATPunchthrough may not always succeed.\n");
			}

			sampleResult = SUCCEEDED;
		}
	}
	virtual void Update(RakNet::RakPeerInterface *rakPeer)
	{
		if (sampleResult == FAILED) return;

		if (sampleResult == PENDING && RakNet::GetTimeMS() > timeout)
		{
			printf("No response from the server, probably not running NatTypeDetectionServer plugin.\n");
			sampleResult = FAILED;
		}
	}
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer)
	{
		delete ntdc;
		ntdc = 0;
	}

	RakNet::NatTypeDetectionClient *ntdc;
	RakNet::TimeMS timeout;
};



struct NatPunchthoughClientFramework : public SampleClientFramework, public RakNet::NatPunchthroughDebugInterface_Printf
{
	RakNet::SystemAddress serverAddress;

	// Set to FAILED to skip this test
	NatPunchthoughClientFramework() { sampleResult = SUPPORT_NAT_PUNCHTHROUGH; npClient = 0; }
	virtual const char * QueryName(void) { return "NatPunchthoughClientFramework"; }
	virtual bool QueryRequiresServer(void) { return true; }
	virtual const char * QueryFunction(void) { return "Causes two systems to try to connect to each other at the same\ntime, to get through routers."; }
	virtual const char * QuerySuccess(void) { return "We can now communicate with the other system, including connecting."; }
	virtual bool QueryQuitOnSuccess(void) { return true; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer)
	{
		if (sampleResult == FAILED) return;

		serverAddress = SelectAmongConnectedSystems(rakPeer, "NatPunchthroughServer");
		if (serverAddress == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
		{
			serverAddress = ClientConnectBlocking(rakPeer, "NatPunchthroughServer", DEFAULT_SERVER_ADDRESS, DEFAULT_SERVER_PORT);
			if (serverAddress == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			{
				printf("Failed to connect to a server.\n");
				sampleResult = FAILED;
				return;
			}
		}

		npClient = new RakNet::NatPunchthroughClient;
		npClient->SetDebugInterface(this);
		rakPeer->AttachPlugin(npClient);


		char guid[128];
		printf("Enter RakNetGuid of the remote system, which should have already connected\nto the server.\nOr press enter to just listen.\n");
		Gets(guid, sizeof(guid));
		if (guid[0])
		{
			RakNet::RakNetGUID remoteSystemGuid;
			remoteSystemGuid.FromString(guid);
			npClient->OpenNAT(remoteSystemGuid, serverAddress);
			isListening = false;

			timeout = RakNet::GetTimeMS() + 10000;
		}
		else
		{
			printf("Listening\n");
			printf("My GUID is %s\n", rakPeer->GetMyGUID().ToString());
			isListening = true;

			// Find the stride of our router in advance
			npClient->FindRouterPortStride(serverAddress);

		}
	}

	virtual void ProcessPacket(RakNet::Packet *packet)
	{
		if (
			packet->data[0] == ID_NAT_TARGET_NOT_CONNECTED ||
			packet->data[0] == ID_NAT_TARGET_UNRESPONSIVE ||
			packet->data[0] == ID_NAT_CONNECTION_TO_TARGET_LOST ||
			packet->data[0] == ID_NAT_PUNCHTHROUGH_FAILED
			)
		{
			RakNet::RakNetGUID guid;
			if (packet->data[0] == ID_NAT_PUNCHTHROUGH_FAILED)
			{
				guid = packet->guid;
			}
			else
			{
				RakNet::BitStream bs(packet->data, packet->length, false);
				bs.IgnoreBytes(1);
				bool b = bs.Read(guid);
				RakAssert(b);
			}

			switch (packet->data[0])
			{
			case ID_NAT_TARGET_NOT_CONNECTED:
				printf("Failed: ID_NAT_TARGET_NOT_CONNECTED\n");
				break;
			case ID_NAT_TARGET_UNRESPONSIVE:
				printf("Failed: ID_NAT_TARGET_UNRESPONSIVE\n");
				break;
			case ID_NAT_CONNECTION_TO_TARGET_LOST:
				printf("Failed: ID_NAT_CONNECTION_TO_TARGET_LOST\n");
				break;
			case ID_NAT_PUNCHTHROUGH_FAILED:
				printf("Failed: ID_NAT_PUNCHTHROUGH_FAILED\n");
				break;
			}

			sampleResult = FAILED;
			return;
		}
		else if (packet->data[0] == ID_NAT_PUNCHTHROUGH_SUCCEEDED)
		{
			unsigned char weAreTheSender = packet->data[1];
			if (weAreTheSender)
				printf("NAT punch success to remote system %s.\n", packet->systemAddress.ToString(true));
			else
				printf("NAT punch success from remote system %s.\n", packet->systemAddress.ToString(true));

			char guid[128];
			printf("Enter RakNetGuid of the remote system, which should have already connected.\nOr press enter to quit.\n");
			Gets(guid, sizeof(guid));
			if (guid[0])
			{
				RakNet::RakNetGUID remoteSystemGuid;
				remoteSystemGuid.FromString(guid);
				npClient->OpenNAT(remoteSystemGuid, serverAddress);

				timeout = RakNet::GetTimeMS() + 10000;
			}
			else
			{
				sampleResult = SUCCEEDED;
			}
		}
	}
	virtual void Update(RakNet::RakPeerInterface *rakPeer)
	{
		if (sampleResult == FAILED) return;

		if (sampleResult == PENDING && RakNet::GetTimeMS() > timeout && isListening == false)
		{
			printf("No response from the server, probably not running NatPunchthroughServer plugin.\n");
			sampleResult = FAILED;
		}
	}
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer)
	{
		delete npClient;
		npClient = 0;
	}

	RakNet::NatPunchthroughClient *npClient;
	RakNet::TimeMS timeout;
	bool isListening;
};


struct Router2Framework : public SampleClientFramework
{
	// Set to FAILED to skip this test
	Router2Framework() { sampleResult = SUPPORT_ROUTER2; router2 = 0; }
	virtual const char * QueryName(void) { return "Router2Framework"; }
	virtual bool QueryRequiresServer(void) { return false; }
	virtual const char * QueryFunction(void) { return "Connect to a peer we cannot directly connect to using the\nbandwidth of a shared peer."; }
	virtual const char * QuerySuccess(void) { return "Router2 assumes we will now connect to the other system."; }
	virtual bool QueryQuitOnSuccess(void) { return true; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer)
	{
		if (sampleResult == FAILED) return;

		printf("Given your application's bandwidth, how much traffic can be forwarded through a single peer?\nIf you use more than half the available bandwidth, then this plugin won't work for you.\n");;
		char supportedStr[64];
		do
		{
			printf("Enter a number greater than or equal to 0: ");
			Gets(supportedStr, sizeof(supportedStr));
		} while (supportedStr[0] == 0);
		int supported = atoi(supportedStr);
		if (supported <= 0)
		{
			printf("Aborting Router2\n");
			sampleResult = FAILED;
			return;
		}

		RakNet::SystemAddress peerAddress = SelectAmongConnectedSystems(rakPeer, "shared peer");
		if (peerAddress == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
		{
			peerAddress = ClientConnectBlocking(rakPeer, "shared peer", "", RAKPEER_PORT_STR);
			if (peerAddress == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			{
				printf("Failed to connect to a shared peer.\n");
				sampleResult = FAILED;
				return;
			}
		}

		char guid[64];
		printf("Destination system must be connected to the shared peer.\n");
		do
		{
			printf("Enter RakNetGUID of destination system: ");
			Gets(guid, sizeof(guid));
		} while (guid[0] == 0);
		RakNet::RakNetGUID endpointGuid;
		endpointGuid.FromString(guid);
		router2 = new RakNet::Router2;
		rakPeer->AttachPlugin(router2);
		router2->EstablishRouting(endpointGuid);

		timeout = RakNet::GetTimeMS() + 5000;
	}
	virtual void ProcessPacket(RakNet::Packet *packet)
	{
	}
	virtual void Update(RakNet::RakPeerInterface *rakPeer)
	{
		if (sampleResult == FAILED) return;

		if (sampleResult == PENDING && RakNet::GetTimeMS() > timeout)
		{
			printf("No response from any system, probably not running Router2 plugin.\n");
			sampleResult = FAILED;
		}
	}
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer)
	{
		delete router2;
		router2 = 0;
	}
	RakNet::Router2 *router2;
	RakNet::TimeMS timeout;
};

struct UDPProxyClientFramework : public SampleClientFramework, public RakNet::UDPProxyClientResultHandler
{
	// Set to FAILED to skip this test
	UDPProxyClientFramework() { sampleResult = SUPPORT_UDP_PROXY; udpProxy = 0; }
	virtual const char * QueryName(void) { return "UDPProxyClientFramework"; }
	virtual bool QueryRequiresServer(void) { return true; }
	virtual const char * QueryFunction(void) { return "Connect to a peer using a shared server connection."; }
	virtual const char * QuerySuccess(void) { return "We can now communicate with the other system, including connecting, within 5 seconds."; }
	virtual bool QueryQuitOnSuccess(void) { return false; }
	virtual void Init(RakNet::RakPeerInterface *rakPeer)
	{
		if (sampleResult == FAILED) return;

		RakNet::SystemAddress serverAddress = SelectAmongConnectedSystems(rakPeer, "UDPProxyCoordinator");
		if (serverAddress == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
		{
			serverAddress = ClientConnectBlocking(rakPeer, "UDPProxyCoordinator", DEFAULT_SERVER_ADDRESS, DEFAULT_SERVER_PORT);
			if (serverAddress == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			{
				printf("Failed to connect to a server.\n");
				sampleResult = FAILED;
				return;
			}
		}
		udpProxy = new RakNet::UDPProxyClient;
		rakPeer->AttachPlugin(udpProxy);
		udpProxy->SetResultHandler(this);

		char guid[128];
		printf("Enter RakNetGuid of the remote system, which should have already connected\nto the server.\nOr press enter to just listen.\n");
		Gets(guid, sizeof(guid));
		RakNet::RakNetGUID targetGuid;
		targetGuid.FromString(guid);

		if (guid[0])
		{
			RakNet::RakNetGUID remoteSystemGuid;
			remoteSystemGuid.FromString(guid);
			udpProxy->RequestForwarding(serverAddress, RakNet::UNASSIGNED_SYSTEM_ADDRESS, targetGuid, UDP_FORWARDER_MAXIMUM_TIMEOUT, 0);
			isListening = false;
		}
		else
		{
			printf("Listening\n");
			printf("My GUID is %s\n", rakPeer->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
			isListening = true;
		}

		timeout = RakNet::GetTimeMS() + 5000;
	}
	virtual void ProcessPacket(RakNet::Packet *packet)
	{
	}
	virtual void Update(RakNet::RakPeerInterface *rakPeer)
	{
		if (sampleResult == FAILED) return;

		if (sampleResult == PENDING && RakNet::GetTimeMS() > timeout && isListening == false)
		{
			printf("No response from the server, probably not running UDPProxyCoordinator plugin.\n");
			sampleResult = FAILED;
		}
	}
	virtual void Shutdown(RakNet::RakPeerInterface *rakPeer)
	{
		delete udpProxy;
		udpProxy = 0;
	}

	virtual void OnForwardingSuccess(const char *proxyIPAddress, unsigned short proxyPort,
		RakNet::SystemAddress proxyCoordinator, RakNet::SystemAddress sourceAddress, RakNet::SystemAddress targetAddress, RakNet::RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
	{
		printf("Datagrams forwarded by proxy %s:%i to target %s.\n", proxyIPAddress, proxyPort, targetAddress.ToString(false));
		printf("Connecting to proxy, which will be received by target.\n");
		RakNet::ConnectionAttemptResult car = proxyClientPlugin->GetRakPeerInterface()->Connect(proxyIPAddress, proxyPort, 0, 0);
		RakAssert(car == CONNECTION_ATTEMPT_STARTED);
		sampleResult = SUCCEEDED;
	}
	virtual void OnForwardingNotification(const char *proxyIPAddress, unsigned short proxyPort,
		RakNet::SystemAddress proxyCoordinator, RakNet::SystemAddress sourceAddress, RakNet::SystemAddress targetAddress, RakNet::RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
	{
		printf("Source %s has setup forwarding to us through proxy %s:%i.\n", sourceAddress.ToString(false), proxyIPAddress, proxyPort);

		sampleResult = SUCCEEDED;
	}
	virtual void OnNoServersOnline(RakNet::SystemAddress proxyCoordinator, RakNet::SystemAddress sourceAddress, RakNet::SystemAddress targetAddress, RakNet::RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
	{
		printf("Failure: No servers logged into coordinator.\n");
		sampleResult = FAILED;
	}
	virtual void OnRecipientNotConnected(RakNet::SystemAddress proxyCoordinator, RakNet::SystemAddress sourceAddress, RakNet::SystemAddress targetAddress, RakNet::RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
	{
		printf("Failure: Recipient not connected to coordinator.\n");
		sampleResult = FAILED;
	}
	virtual void OnAllServersBusy(RakNet::SystemAddress proxyCoordinator, RakNet::SystemAddress sourceAddress, RakNet::SystemAddress targetAddress, RakNet::RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
	{
		printf("Failure: No servers have available forwarding ports.\n");
		sampleResult = FAILED;
	}
	virtual void OnForwardingInProgress(const char *proxyIPAddress, unsigned short proxyPort, RakNet::SystemAddress proxyCoordinator, RakNet::SystemAddress sourceAddress, RakNet::SystemAddress targetAddress, RakNet::RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)
	{
		printf("Notification: Forwarding already in progress.\n");
	}

	RakNet::UDPProxyClient *udpProxy;
	RakNet::TimeMS timeout;
	bool isListening;
};
