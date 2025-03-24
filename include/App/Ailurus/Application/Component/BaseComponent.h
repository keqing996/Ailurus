#pragma once

#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
    REFLECTION_ENUM(ComponentType,
        Transform,
        MeshRender,
        Light)

    class Component: public NonCopyable
    {
    public:
    };
}