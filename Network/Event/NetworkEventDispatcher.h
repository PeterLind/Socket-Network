#pragma once
#include <Function.h>
#include "SharedDefines.h"
#include <any>
#include <functional>
#include <mutex>

namespace FOC
{
	namespace NETWORK
	{
		class NetworkEventDispatcher
		{
		public:
			NetworkEventDispatcher() = default;
			~NetworkEventDispatcher() = default;

			template<class T>
			void Subscribe(PackageType aType, std::function<void(T)> aFunction);
			void Subscribe(PackageType aType, std::function<void()> aFunction);
			void Execute();
			template<class T>
			void AddForExecute(PackageType aType, T aValue);
		private:
			std::unordered_map<PackageType, std::vector<std::function<void(std::any)>>> mySubscribers;
			std::vector<std::pair<PackageType, std::any>> myListOfExectutions;
			std::mutex myLock;
		};
		template<class T>
		inline void NetworkEventDispatcher::Subscribe(PackageType aType, std::function<void(T)> aFunction)
		{
			auto wrapper = [aFunction](std::any data)
				{
					if(data.type() == typeid(T))
					{
						aFunction(std::any_cast<T>(data));
					}
					else
					{
						std::cout << "Something went wrong" << std::endl;
					}
				};
			mySubscribers[aType].push_back(wrapper);
		}

		inline void NetworkEventDispatcher::Subscribe(PackageType aType, std::function<void()> aFunction)
		{
			auto wrapper = [aFunction](std::any data) {
					aFunction();
				};
			mySubscribers[aType].push_back(wrapper);
		}

		template<class T>
		inline void NetworkEventDispatcher::AddForExecute(PackageType aType, T aValue)
		{
			std::lock_guard<std::mutex> lock(myLock);
			char* rat = new char[PACKAGE_DATA_SIZE];
			memcpy(rat, aValue, PACKAGE_DATA_SIZE);
			myListOfExectutions.push_back(std::make_pair(aType, rat));
			aValue;
		}

		inline void NetworkEventDispatcher::Execute()
		{
			std::lock_guard<std::mutex> lock(myLock);
			for (size_t i = 0; i < myListOfExectutions.size(); i++)
			{
				for(auto& subsriber : mySubscribers[myListOfExectutions[i].first])
				{
					subsriber(myListOfExectutions[i].second);
				}
				delete std::any_cast<char*>(myListOfExectutions[i].second);
			}
			myListOfExectutions.clear();
		}
	}
}

