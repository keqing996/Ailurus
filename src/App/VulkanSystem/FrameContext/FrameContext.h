#pragma once

#include <cstdint>
#include <optional>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	struct RenderingFrameContext
	{
		uint64_t renderingFrameCount;
		std::vector<std::pair<vk::CommandBuffer, vk::Semaphore>> usingResources;
		vk::Fence allFinishFence;
	};

	struct FrameContext
	{
		std::optional<RenderingFrameContext> renderingFrameContext;
		std::vector<std::pair<vk::CommandBuffer, vk::Semaphore>> waitingSubmittedCmdBuffers;
		vk::Semaphore imageReadySemaphore;
		uint64_t lastRenderFinishedFrame;
	};
} // namespace Ailurus