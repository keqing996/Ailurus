#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    struct Flight
    {
        vk::CommandBuffer commandBuffer;
        vk::Semaphore imageReadySemaphore;
        vk::Semaphore renderFinishSemaphore;
        vk::Fence fence;
    };
}