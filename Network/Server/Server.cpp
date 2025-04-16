#pragma once
#include "network_pch.h"
#include "Server.h"
#include "NetworkApplication.h"
#include "NetworkEventDispatcher.h"
#include <random>


namespace FOC
{
	namespace NETWORK
	{
		Server::Server(NetworkApplication* aApplication) : myApplication(aApplication)
		{
		}
		bool Server::Init()
		{
			addrinfo hints;

			std::cout << "Starting Winsock...";
			if(WSAStartup(MAKEWORD(2, 2), &myInformation.myWinsockData) != 0)
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
			if(myInformation.mySocket == INVALID_SOCKET)
			{
				std::cout << "Failed to create socket." << std::endl;
				std::cout << "Error: " << WSAGetLastError() << std::endl;
				WSACleanup();
				return EXIT_FAILURE;
			}


			myInformation.myAddrServer.sin_family = AF_INET; // IP4 Config - Same as hits
			myInformation.myAddrServer.sin_addr.s_addr = htonl(INADDR_ANY); // Server Doesn't care where the information is comming from so it allows any information 
			myInformation.myAddrServer.sin_port = htons(SERVER_PORT);   // Lock Server to a Specific Port

			// We need to bind to a specific address and port combination. This tells the
			// operating system that we want communication on that combination to go to
			// this program.
			if (bind(myInformation.mySocket, reinterpret_cast<sockaddr*>(&myInformation.myAddrServer), sizeof(myInformation.myAddrServer)) == SOCKET_ERROR)
			{
				std::cout << "Failed to bind socket." << std::endl;
				std::cout << "Error: " << WSAGetLastError() << std::endl;
				return EXIT_FAILURE;
			}
			return true;
		}
		void Server::Update()
		{
			ZeroMemory(&myMessageFrom, NETMESSAGE_SIZE);
			ClientInfomation ReceiveClient;
			const int recv_len = recvfrom(myInformation.mySocket, (char*)&myMessageFrom, NETMESSAGE_SIZE, 0, (sockaddr *)&ReceiveClient.myAddr, &ReceiveClient.myAddrSize);
			const int ErrorCode = WSAGetLastError();
			if(recv_len == SOCKET_ERROR && ErrorCode != WSAEWOULDBLOCK)
			{
				std::cout << "Failed receiving data from udpSocket." << std::endl;
				std::cout << "Error: " << WSAGetLastError() << std::endl;
			}
			if(recv_len > 0)
			{
				ParseMessage(ReceiveClient);
			}
		}
		void Server::SendTo(const char aMessage[NETMESSAGE_SIZE])
		{
			for(size_t i = 0; i < clientptr; i++)
			{
				if(sendto(myInformation.mySocket, aMessage, NETMESSAGE_SIZE, 0, (sockaddr *)&myClients[i].myAddr, myClients[i].myAddrSize) == SOCKET_ERROR) 
				{
					std::cout << "Error Sending Message" << std::endl;
					return;
				} // Server Sends to each client to update them with information
			}
		}
		void Server::ParseMessage(ClientInfomation &aClient)
		{
			if(myMessageFrom.myType & static_cast<unsigned char>(PackageType::eCONNECT))
			{
				myClients[clientptr].myAddr.sin_family = aClient.myAddr.sin_family;
				myClients[clientptr].myAddr.sin_addr = aClient.myAddr.sin_addr;
				myClients[clientptr].myAddr.sin_port = aClient.myAddr.sin_port;
				myClients[clientptr].myId = std::atoi(myMessageFrom.message);
				myMessageTo.myType = static_cast<unsigned char>(PackageType::eCONNECT);
				std::strcpy(myMessageTo.message, "ACK");
				sendto(myInformation.mySocket, (char*)&myMessageTo, NETMESSAGE_SIZE, 0, (sockaddr *)&myClients[clientptr].myAddr, myClients[clientptr].myAddrSize);
				clientptr++;
			}
			if (myMessageFrom.myType & static_cast<unsigned char>(PackageType::ePLAYER_ACTION))
			{
				myApplication->myEventDispatcher->AddForExecute<char*>(PackageType::ePLAYER_ACTION, myMessageFrom.message);
			}
			if (myMessageFrom.myType & static_cast<unsigned char>(PackageType::eACK))
			{
				myApplication->myEventDispatcher->AddForExecute<char*>(PackageType::eACK, myMessageFrom.message);
			}
		}
	}
}