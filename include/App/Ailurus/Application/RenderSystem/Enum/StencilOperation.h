#pragma once

#include <Ailurus/Utility/EnumReflection.h>

namespace Ailurus
{
    REFLECTION_ENUM(StencilLoadType, 
        None,
        Clear,
        Load);

    REFLECTION_ENUM(StencilWriteType, 
        None,
        Write);
}