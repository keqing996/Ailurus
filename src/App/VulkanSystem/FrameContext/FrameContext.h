#pragma once

#include <cstdint>
#include <optional>
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
		std::optional<uint64_t> renderingFrame;
		uint64_t lastRenderFinishedFrame;

    public:
		FrameContext();
		~FrameContext();
	};
} // namespace Ailurus