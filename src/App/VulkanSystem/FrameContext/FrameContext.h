#pragma once

#include <cstdint>
#include <optional>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	struct RenderingFrameContext
	{
		uint64_t renderingFrameCount;
		vk::CommandBuffer renderingCommandBuffer;
		std::vector<vk::Semaphore> usingSemaphores;
		vk::Fence allFinishFence;
	};

	struct FrameContext
	{
		std::optional<RenderingFrameContext> renderingFrameContext;
		std::vector<vk::Semaphore> beforeRenderSemaphores;
		uint64_t lastRenderFinishedFrame;
	};
} // namespace Ailurus