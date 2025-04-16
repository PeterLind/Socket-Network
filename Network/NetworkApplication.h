#pragma once
#include "SharedDefines.h"
#include <Function.h>

namespace FOC
{
	namespace NETWORK
	{
		class Server;
		class Client;
		class NetworkEventDispatcher;
		class NetworkApplication
		{
		public:
			NetworkApplication();
			~NetworkApplication();

			bool CreateServer();
			bool CreateClient(unsigned int someOctates[4]);
			
			void UpdateMessage();
			void SendTo(char* aMessage);

			const bool IsConnected() const;
			const Server* GetServer() const;
			const Client* GetClient() const;
			
			int* GetUserIP();
			NetworkEventDispatcher *GetEventDispatcher();
		private:
			//This is a cool way of allowing Server and Client to be seperate classes but still allow them to update
			std::function<void()> myUpdateFunction = nullptr;
			std::function<void(char* aMessage)> mySendToFunction = nullptr;
			
			std::unique_ptr<NetworkEventDispatcher> myEventDispatcher; // Make it unique because only NetworkApplcation should hold this
			Server *myServer = nullptr;
			Client *myClient = nullptr;
			int *userIPandPort;
			friend Client;
			friend Server;
		};
	}
}

