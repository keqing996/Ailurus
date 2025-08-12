#include "VulkanSemaphore.h"
#include <VulkanContext/VulkanContext.h>

namespace Ailurus
{
    VulkanSemaphore::VulkanSemaphore()
    {
        vk::SemaphoreCreateInfo semaphoreInfo;
        _vkSemaphore = VulkanContext::GetDevice().createSemaphore(semaphoreInfo);
    }

    VulkanSemaphore::~VulkanSemaphore()
    {
        VulkanContext::GetDevice().destroySemaphore(_vkSemaphore);
    }

    vk::Semaphore VulkanSemaphore::GetSemaphore() const
    {
        return _vkSemaphore;
    }
}