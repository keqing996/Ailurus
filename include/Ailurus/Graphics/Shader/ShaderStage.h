#pragma once

#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
    REFLECTION_ENUM(ShaderStage,
        Vertex,
        Fragment
    )

    class Shader;
    using PipelineShaderStages = std::array<const Shader*, EnumReflection<ShaderStage>::Size()>;

    namespace ShaderStageUtil
    {
        vk::ShaderStageFlagBits ToVulkanEnum(ShaderStage stage);
    }

}