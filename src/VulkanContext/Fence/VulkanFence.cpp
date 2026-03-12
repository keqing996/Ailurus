#include "VulkanFence.h"
#include <VulkanContext/VulkanContext.h>
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    VulkanFence::VulkanFence(bool initSignaled)
    {
        vk::FenceCreateInfo fenceInfo;

        if (initSignaled)
            fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        try
        {
            _vkFence = VulkanContext::GetDevice().createFence(fenceInfo);
        }
        catch (const vk::SystemError& e)
        {
            Logger::LogError("Failed to create fence: {}", e.what());
        }
    }

    VulkanFence::~VulkanFence()
    {
        try
        {
            VulkanContext::GetDevice().destroyFence(_vkFence);
        }
        catch (const vk::SystemError& e)
        {
            Logger::LogError("Failed to destroy fence: {}", e.what());
        }
    }

    const vk::Fence& VulkanFence::GetFence() const
    {
        return _vkFence;
    }

	void VulkanFence::Reset()
	{
        try
        {
    	    VulkanContext::GetDevice().resetFences(_vkFence);
        }
        catch (const vk::SystemError& e)
        {
            Logger::LogError("Failed to reset fence: {}", e.what());
        }
	}
} // namespace Ailurus