#pragma once

#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
    REFLECTION_ENUM(AssetType,
        Model,
        Material,
        MaterialInstance,
        Texture
    );
} // namespace Ailurus