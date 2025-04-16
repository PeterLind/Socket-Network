#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cassert>
#include <memory>
#include <Windows.h>
#include <string>

#define NETMESSAGE_SIZE 512
#define LOOPBACK_ADDRESS "127.0.0.1"
// Ports 49152-65535– These are used by client programs and you are free to use these in client programs
#define SERVER_PORT 57289 // I'm using this because I like the Number :) -- Peter

namespace FOC
{
	namespace NETWORK
	{
        enum class PackageType : unsigned char //Need to add more
        {
            eCONNECT =  1 << 0,
            eDISCONNECT = 1 << 1,
            ePLAYER_ACTION = 1 << 2,
            eENEMY_ACTION = 1 << 3,
            eSTARTGAME = 1 << 4,
            eACK = 1 << 5,
            eSPAWNOBJECT = 1 << 6,
  
        };

		struct NetworkConnectionInfo
		{
			SOCKET mySocket;
			// https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html, mmmm yes
			struct sockaddr_in myAddrServer; // why is this a struct? Fuck if I know, but without it the code won't work. Something with sockaddr_in* != sockaddr_in* <- Wack -- Peter
			WSADATA myWinsockData;
            socklen_t myAddrSize = sizeof(sockaddr_in);
            char myId;
		};
        const size_t PACKAGE_DATA_SIZE = NETMESSAGE_SIZE - sizeof(PackageType);

        struct MessagePackage
        {
            MessagePackage() = default;
            MessagePackage(PackageType aType) : myType(static_cast<unsigned char>(aType)) {};
            ~MessagePackage() = default;
            operator char* () const {
                    //Code here
                static char fullMessage[NETMESSAGE_SIZE];
                ZeroMemory(fullMessage, NETMESSAGE_SIZE);
                memcpy(fullMessage, &myType, sizeof(unsigned char));
                memcpy(fullMessage + sizeof(unsigned char), message, sizeof(message));
                return fullMessage;
            }

            unsigned char myType; // Why it's a unsigned char instead of a a Package Type is because then can I align to check message types 
            char message[PACKAGE_DATA_SIZE];
        };


		class DataBuilder
		{
        public:
            DataBuilder() = default;
            DataBuilder(MessagePackage& aPackage) : myMessage(aPackage.message) {};
            ~DataBuilder() = default;

            //You need to pre-declare the message being used for using this function
            template<typename T>
            DataBuilder& Write(const T* data)
            {
                return Write(myMessage, data, sizeof(T));
            }

            //You need to pre-declare the message being used for using this function
            template<typename T>
            DataBuilder& Write(const T data)
            {
                return Write(myMessage, &data, sizeof(T));
            }

            template<typename T>
            DataBuilder &Write(void *destination, const T *data)
            {
                return Write(destination, data, sizeof(T));
            }

            template<typename T>
            DataBuilder& Write(void* destination, const T data)
            {
                return Write(destination, &data, sizeof(T));
            }
            void WriteRemaining(void *destination, const void *data)
            {
                Write(destination, data, GetRemainingData());
            }

            template<typename T>
            DataBuilder& Read(T* destination, const void* data)
            {
                assert(myReadCurrentSize + sizeof(T) <= PACKAGE_DATA_SIZE);

                memcpy(destination, (char*)data + myReadCurrentSize, sizeof(T));
                myReadCurrentSize += sizeof(T);

                return *this;
            }
        private:
            size_t GetRemainingData() const
            {
                return PACKAGE_DATA_SIZE - myWriteCurrentSize;
            }
            DataBuilder &Write(void *destination, const void *data, size_t size)
            {
                assert(myWriteCurrentSize + size <= PACKAGE_DATA_SIZE);

                memcpy((char *)destination + myWriteCurrentSize, data, size);
                myWriteCurrentSize += size;

                return *this;
            }
            void* myMessage;

            size_t myWriteCurrentSize = 0;
            size_t myReadCurrentSize = 0;
		};
	}
}

