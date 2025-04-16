 #pragma once
#include "network_pch.h"
#include "NetworkApplication.h"
#include "Server.h"
#include "Client.h"
#include "NetworkEventDispatcher.h"

namespace FOC
{
	namespace NETWORK
	{
		NetworkApplication::NetworkApplication() : myEventDispatcher(std::make_unique<NetworkEventDispatcher>())
		{
			userIPandPort = new int[4] {0, 0, 0, 0};
			char hostName[1024];
			addrinfo hints, * result;
			WSADATA wsaData;
			WORD verisonRequest = MAKEWORD(2, 2);

			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_DGRAM;
			hints.ai_protocol = IPPROTO_UDP;

			if (WSAStartup(verisonRequest, &wsaData) != 0) {};
			if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR)
			{
				WSACleanup();
			}

			if (getaddrinfo(hostName, NULL, &hints, &result) != NULL)
			{
				WSACleanup();
			}
			struct in_addr* addr = &((struct sockaddr_in*)result->ai_addr)->sin_addr;
			userIPandPort[0] = addr->S_un.S_un_b.s_b1;
			userIPandPort[1] = addr->S_un.S_un_b.s_b2;
			userIPandPort[2] = addr->S_un.S_un_b.s_b3;
			userIPandPort[3] = addr->S_un.S_un_b.s_b4;
			freeaddrinfo(result);
			WSACleanup();
		}
		NetworkApplication::~NetworkApplication()
		{
			delete userIPandPort;
		}
		bool NetworkApplication::CreateServer()
		{
			myServer = new Server(this);
			myUpdateFunction = [this]() { myServer->Update(); }; // Changes Functionens behaviour
			mySendToFunction = [this](char* aMessage) { myServer->SendTo(aMessage); }; // Changes Functionens behaviour
			return myServer->Init();
		}

		bool NetworkApplication::CreateClient(unsigned int someOctates[4])
		{
			myClient = new Client(this);
			myUpdateFunction = [this]() { myClient->Update(); }; // Changes Functionens behaviour
			mySendToFunction = [this](char* aMessage) { myClient->SendTo(aMessage); }; // Changes Functionens behaviour
			return myClient->Init(someOctates);
		}

		void NetworkApplication::UpdateMessage()
		{
			if (myUpdateFunction)
			{
				while (true)
				{
					myUpdateFunction(); // kallar på functionen 
				}
			}
		}

		void NetworkApplication::SendTo(char* aMessage)
		{
			if (mySendToFunction) 
			{
				mySendToFunction(aMessage);
			}
		}

		int* NetworkApplication::GetUserIP()
		{
			return userIPandPort;
		}

		NetworkEventDispatcher* NetworkApplication::GetEventDispatcher()
		{
			return myEventDispatcher.get();
		}
		const bool NetworkApplication::IsConnected() const
		{
			return GetServer() || GetClient();
		}

		const Server* NetworkApplication::GetServer() const 
		{
			return myServer;
		}
		const Client* NetworkApplication::GetClient() const
		{
			return myClient;
		}

	}
}
