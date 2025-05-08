#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>
#include "VulkanSystem/CommandBuffer/VulkanCommandBuffer.h"

namespace Ailurus
{
	struct FrameContext
	{
		VulkanCommandBuffer commandBuffer;
		vk::Semaphore imageReadySemaphore;
		vk::Semaphore renderFinishSemaphore;
		vk::Fence fence;
		uint64_t renderingFrame;

    public:
		FrameContext();
		~FrameContext();
	};
} // namespace Ailurus