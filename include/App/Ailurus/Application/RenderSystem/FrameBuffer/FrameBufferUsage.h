#pragma once

#include <Ailurus/Utility/EnumReflection.h>

namespace Ailurus
{
    REFLECTION_ENUM(FrameBufferUsage, 
        PresentBackBuffer, 
        GBuffer,
        ShadowMap);
}