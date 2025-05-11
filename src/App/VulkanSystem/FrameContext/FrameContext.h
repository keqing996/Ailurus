#pragma once

#include <cstdint>
#include <optional>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	struct RecordedCommandBufferInfo
	{
		std::optional<vk::Semaphore> waitSemaphore;
		vk::CommandBuffer buffer;
		vk::Semaphore signalSemaphore;
		std::vector<vk::PipelineStageFlags> waitStages;
	};

	struct RenderingFrameContext
	{
		uint64_t renderingFrameCount;
		std::vector<vk::CommandBuffer> renderingBuffers;
		std::unordered_set<vk::Semaphore> usingSemaphores;
		vk::Fence allFinishFence;
	};

	struct FrameContext
	{
		// Current rendering (not fenced) frame context
		std::optional<RenderingFrameContext> renderingFrameContext;

		// Recorded non-render command buffer and related semaphores
		std::vector<RecordedCommandBufferInfo> waitingSubmittedCmdBuffers;

		uint64_t lastRenderFinishedFrame;

	public:
		bool WaitFinish();
		void AddCommandBuffer(vk::CommandBuffer buffer, vk::Semaphore waitSemaphore = nullptr, std::vector<vk::PipelineStageFlags> waitStages = {});
	};
} // namespace Ailurus