#pragma once

#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
    REFLECTION_ENUM(RenderPassType,
        Forward,
        ImGui,
        Present,
    );
}