#pragma once

#include "VulkanContext/VulkanPch.h"
#include <Ailurus/Utility/NonCopyable.h>

namespace Ailurus
{
    class VulkanFence: public NonCopyable
    {
    public:
        explicit VulkanFence(bool initSignaled);
        ~VulkanFence();

        const vk::Fence& GetFence() const;
    	void Reset();

    private:
        vk::Fence _vkFence;
        
    };
}