#pragma once

#include <Ailurus/Utility/EnumReflection.h>

namespace Ailurus
{
    REFLECTION_ENUM(MultiSamplingType, 
        None, 
        MSAAx2, 
        MSAAx4, 
        MSAAx8, 
        MSAAx16, 
        MSAAx32, 
        MSAAx64);
}