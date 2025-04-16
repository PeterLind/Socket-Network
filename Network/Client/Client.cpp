#pragma once
#include "network_pch.h"
#include "Client.h"
#include "NetworkApplication.h"
#include "NetworkEventDispatcher.h"
#include <random>

namespace FOC
{
	namespace NETWORK
	{
		Client::Client(NetworkApplication* aApplication) : myApplication(aApplication)
		{
		}
		bool Client::Init(unsigned int someOctates[4])
		{
			someOctates;
#ifndef LOOPBACK_ADDRESS
			unsigned int ip = 0;
			for (int i = 3; i >= 0; i--)
			{
				ip <<= 8u;
				ip += someOctates[i];
			}
#endif
			addrinfo hints;

			std::cout << "Starting Winsock...";
			if (WSAStartup(MAKEWORD(2, 2), &myInformation.myWinsockData) != 0)
			{
				std::cout << " FAIL!" << std::endl;
				std::cout << "Error: " << WSAGetLastError() << std::endl;
				return EXIT_FAILURE;
			}
			std::cout << "OK!" << std::endl;


			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET; // Could use AF_INET6 for a IPv6 development, but I am still learning so this will do
			hints.ai_socktype = SOCK_DGRAM;
			hints.ai_protocol = IPPROTO_UDP;


			myInformation.mySocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
			if (myInformation.mySocket == INVALID_SOCKET)
			{
				std::cout << "Failed to create socket." << std::endl;
				std::cout << "Error: " << WSAGetLastError() << std::endl;
				WSACleanup();
				return EXIT_FAILURE;
			}

			// Set udpSocket to non blocking
			// This way recvfrom will not wait until it gets a message
			u_long iMode = true;
			ioctlsocket(myInformation.mySocket, FIONBIO, &iMode);

			myInformation.myAddrServer.sin_family = AF_INET;
#ifdef LOOPBACK_ADDRESS
			InetPton(AF_INET, TEXT(LOOPBACK_ADDRESS), &myInformation.myAddrServer.sin_addr.s_addr);
#else
			myInformation.myAddrServer.sin_addr.s_addr = ip;
#endif
			myInformation.myAddrServer.sin_port = htons(SERVER_PORT);
			SendTo("\1");
			return EXIT_SUCCESS;
		}
		void Client::Update()
		{
			const int recv_len = ReceiveFrom(); // This Pauses the loop untill it's received a message
			const int ErrorCode = WSAGetLastError();
			if (recv_len == SOCKET_ERROR && ErrorCode != WSAEWOULDBLOCK)
			{
				std::cout << "Failed receiving data from udpSocket." << std::endl;
				std::cout << "Error: " << WSAGetLastError() << std::endl;
			}
			if (recv_len > 0)
			{
				ParseMessage();
			}

		}
		void Client::ParseMessage()
		{
			if (myMessageFrom.myType & static_cast<unsigned char>(PackageType::eCONNECT)) // This is cancer, different way? Function ptrs? Investigate
			{
				myIsConnected = true;
			}
			if (myMessageFrom.myType & static_cast<unsigned char>(PackageType::eDISCONNECT))
			{
				myIsConnected = false;
				//Destroy all data, basicly act as a Destroctor
			}
			if (myMessageFrom.myType & static_cast<unsigned char>(PackageType::eSTARTGAME))
			{
				myApplication->myEventDispatcher->AddForExecute<char*>(PackageType::eSTARTGAME, myMessageFrom.message);
			}
			if (myMessageFrom.myType & static_cast<unsigned char>(PackageType::ePLAYER_ACTION))
			{
				myApplication->myEventDispatcher->AddForExecute<char*>(PackageType::ePLAYER_ACTION, myMessageFrom.message);
			}
			if (myMessageFrom.myType & static_cast<unsigned char>(PackageType::eENEMY_ACTION))
			{
				myApplication->myEventDispatcher->AddForExecute<char*>(PackageType::eENEMY_ACTION, myMessageFrom.message);
			}
			if (myMessageFrom.myType & static_cast<unsigned char>(PackageType::eSPAWNOBJECT))
			{
				myApplication->myEventDispatcher->AddForExecute<char*>(PackageType::eSPAWNOBJECT, myMessageFrom.message);
			}
		}

		const bool Client::IsConnected() const
		{
			return myIsConnected;
		}
		const bool Client::GameIsStarted() const
		{
			return myGameIsStarted;
		}
		void Client::SendTo(const char aMessage[NETMESSAGE_SIZE])
		{
			if (sendto(myInformation.mySocket, aMessage, NETMESSAGE_SIZE, 0, (sockaddr*)&myInformation.myAddrServer, myInformation.myAddrSize) == SOCKET_ERROR) // Server Sends to each client to update them with information
			{
				std::cout << "Error Sending Message" << std::endl;
				return;
			}
		}
		int Client::ReceiveFrom()
		{
			ZeroMemory(&myMessageFrom, NETMESSAGE_SIZE);
			return recvfrom(myInformation.mySocket, (char*)&myMessageFrom, NETMESSAGE_SIZE, 0, (sockaddr*)&myInformation.myAddrServer, &myInformation.myAddrSize);
		}
	}
}