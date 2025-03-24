#include "Ailurus/Application/Entity/Entity.h"

namespace Ailurus
{
    uint32_t Entity::GetGuid() const
    {
        return _guid;
    }

    Entity::Entity(uint32_t guid)
        : _guid(guid)
    {
    }
}
