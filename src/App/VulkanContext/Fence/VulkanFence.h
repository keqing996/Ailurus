#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>

namespace Ailurus
{
    class VulkanFence: public NonCopyable
    {
    public:
        explicit VulkanFence(bool initSignaled);
        ~VulkanFence();

        vk::Fence GetFence() const;

    private:
        vk::Fence _vkFence;
        
    };
}