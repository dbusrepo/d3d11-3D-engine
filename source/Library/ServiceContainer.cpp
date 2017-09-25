#include "ServiceContainer.h"

namespace Library
{
	ServiceContainer::ServiceContainer()
		: mServices()
	{
	}

	void ServiceContainer::AddService(uintptr_t typeID, void* service)
	{
		mServices.insert(std::pair<uintptr_t, void*>(typeID, service));
	}

	void ServiceContainer::RemoveService(uintptr_t typeID)
	{
		mServices.erase(typeID);
	}

	void* ServiceContainer::GetService(uintptr_t typeID) const
	{
		std::map<uintptr_t, void*>::const_iterator it = mServices.find(typeID);

		return (it != mServices.end() ? it->second : nullptr);
	}
}
