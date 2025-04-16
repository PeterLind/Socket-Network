#pragma once
#include "SharedDefines.h"

namespace FOC
{
	namespace NETWORK
	{
		class NetworkApplication;
		class Client
		{
		public:
			Client() = default;
			Client(NetworkApplication* aApplication);
			~Client() = default;

			bool Init(unsigned int someOctates[4]);
			void Update();

			const bool IsConnected() const;
			const bool GameIsStarted() const;
			void SendTo(const char aMessage[NETMESSAGE_SIZE]);
		private:
			void ParseMessage();
			int ReceiveFrom();

			NetworkApplication* myApplication;

			bool myIsConnected = false;
			bool myGameIsStarted = false;

			NetworkConnectionInfo myInformation;
			MessagePackage myMessageTo;
			MessagePackage myMessageFrom;
			friend NetworkApplication;
		};
	}
}

