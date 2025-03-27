#include "Ailurus/Application/Entity/Entity.h"

namespace Ailurus
{
    uint32_t Entity::GetGuid() const
	{
		return _guid;
	}

	Component* Entity::GetComponent(ComponentType type) const
	{
		for (const auto& pComp : _components)
		{
			if (Component::IsDerivedFrom(pComp->GetType(), type))
				return pComp.get();
		}

    	return nullptr;
	}

	Entity::Entity(uint32_t guid)
        : _guid(guid)
    {
    }
}
