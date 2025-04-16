#pragma once
#include <basetyps.h>
#include <any>

namespace FOC
{
	namespace NETWORK
	{
		interface NetworkObserver
		{
		public:
			virtual void RecieveEvent(std::any aValue) = 0; // Event based pasing from classes works
		};
	}
}

