#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>

namespace Ailurus
{
    class VulkanSemaphore: public NonCopyable
    {
    public:
        VulkanSemaphore();
        ~VulkanSemaphore();

        const vk::Semaphore& GetSemaphore() const;

    private:
        vk::Semaphore _vkSemaphore;
    };
}