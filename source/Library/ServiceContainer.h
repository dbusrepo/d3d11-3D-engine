#pragma once

#include "Common.h"

namespace Library
{
	class ServiceContainer
	{
	public:
		ServiceContainer();

		void AddService(uintptr_t typeID, void* service);
		void RemoveService(uintptr_t typeID);
		void* GetService(uintptr_t typeID) const;

	private:
		ServiceContainer(const ServiceContainer& rhs);
		ServiceContainer& operator=(const ServiceContainer& rhs);

		std::map<uintptr_t, void*> mServices;
	};
}