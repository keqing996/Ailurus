#include "VulkanSemaphore.h"
#include <VulkanContext/VulkanContext.h>
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    VulkanSemaphore::VulkanSemaphore()
    {
        vk::SemaphoreCreateInfo semaphoreInfo;
        try
        {
            _vkSemaphore = VulkanContext::GetDevice().createSemaphore(semaphoreInfo);
        }
        catch (const vk::SystemError& e)
        {
            Logger::LogError("Failed to create semaphore: {}", e.what());
        }
    }

    VulkanSemaphore::~VulkanSemaphore()
    {
        try
        {
            VulkanContext::GetDevice().destroySemaphore(_vkSemaphore);
        }
        catch (const vk::SystemError& e)
        {
            Logger::LogError("Failed to destroy semaphore: {}", e.what());
        }
    }

    const vk::Semaphore& VulkanSemaphore::GetSemaphore() const
    {
        return _vkSemaphore;
    }
}