#include "VulkanFence.h"
#include <VulkanContext/VulkanContext.h>

namespace Ailurus
{
    VulkanFence::VulkanFence(bool initSignaled)
    {
        vk::FenceCreateInfo fenceInfo;

        if (initSignaled)
            fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        _vkFence = VulkanContext::GetDevice().createFence(fenceInfo);
    }

    VulkanFence::~VulkanFence()
    {
        VulkanContext::GetDevice().destroyFence(_vkFence);
    }

    const vk::Fence& VulkanFence::GetFence() const
    {
        return _vkFence;
    }

	void VulkanFence::Reset()
	{
    	VulkanContext::GetDevice().resetFences(_vkFence);
	}
} // namespace Ailurus