#pragma once
#include "SharedDefines.h"
#include "Client.h"

namespace FOC
{
	namespace NETWORK
	{
		struct NetworkConnectionInfo;
		class NetworkApplication;
		class Server
		{
		private:
			struct ClientInfomation
			{
				sockaddr_in myAddr;
				socklen_t myAddrSize = sizeof(sockaddr_in);
				int myId;
			};
		public:
			Server() = default;
			Server(NetworkApplication* aApplication);
			~Server() = default;

			bool Init();
			void Update();
			void SendTo(const char aMessage[NETMESSAGE_SIZE]);
		private:
			void ParseMessage(ClientInfomation& aClient);

			NetworkApplication* myApplication;

			NetworkConnectionInfo myInformation;

			MessagePackage myMessageTo;
			MessagePackage myMessageFrom;
			std::array<ClientInfomation, 1> myClients;
			unsigned int clientptr = 0;
			friend NetworkApplication;
		};
	}
}

