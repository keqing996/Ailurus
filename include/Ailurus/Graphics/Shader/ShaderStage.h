#pragma once

#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
    REFLECTION_ENUM(ShaderStage,
        Vertex,
        Fragment
    )

    namespace ShaderStageUtil
    {
        vk::ShaderStageFlagBits ToVulkanEnum(ShaderStage stage);
    }

}