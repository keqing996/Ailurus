#include "VulkanFence.h"
#include <VulkanContext/VulkanContext.h>

namespace Ailurus
{
    VulkanFence::VulkanFence()
    {
        vk::FenceCreateInfo fenceInfo;
        _vkFence = VulkanContext::GetDevice().createFence(fenceInfo);
    }

    VulkanFence::~VulkanFence()
    {
        VulkanContext::GetDevice().destroyFence(_vkFence);
    }

    vk::Fence VulkanFence::GetFence() const
    {
        return _vkFence;
    }
}