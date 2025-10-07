#pragma once

#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
    REFLECTION_ENUM(RenderPassType,
        GBuffer,        // Geometry pass - render to G-Buffer
        Lighting,       // Lighting pass - compute lighting from G-Buffer
        Forward,        // Forward pass - for transparent objects
        ImGui
    );
}