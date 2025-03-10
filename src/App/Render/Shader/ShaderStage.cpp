#include "Ailurus/Graphics/Shader/ShaderStage.h"

namespace Ailurus::ShaderStageUtil
{
    vk::ShaderStageFlagBits ToVulkanEnum(ShaderStage stage)
    {
        vk::ShaderStageFlagBits vkStage = vk::ShaderStageFlagBits::eAll;
        switch (stage)
        {
            case ShaderStage::Vertex:
                vkStage = vk::ShaderStageFlagBits::eVertex;
                break;
            case ShaderStage::Fragment:
                vkStage = vk::ShaderStageFlagBits::eFragment;
                break;
        }

        return vkStage;
    }
}
