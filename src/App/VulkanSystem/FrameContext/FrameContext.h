#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_set>
#include <vulkan/vulkan.hpp>
#include "VulkanSystem/CommandBuffer/VulkanCommandBuffer.h"
#include "VulkanSystem/Helper/HashHelper.hpp"

namespace Ailurus
{
	struct RecordedCommandBufferInfo
	{
		std::optional<vk::Semaphore> waitSemaphore;
		std::unique_ptr<VulkanCommandBuffer> pCommandBuffer;
		vk::Semaphore signalSemaphore;
		std::vector<vk::PipelineStageFlags> waitStages;
	};

	struct RenderingFrameContext
	{
		uint64_t renderingFrameCount;
		std::vector<std::unique_ptr<VulkanCommandBuffer>> renderingBuffers;
		VkObjectSet<vk::Semaphore> usingSemaphores;
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
		void AddCommandBuffer(std::unique_ptr<VulkanCommandBuffer>&& pBuffer);
		void AddCommandBuffer(std::unique_ptr<VulkanCommandBuffer>&& pBuffer, vk::Semaphore waitSemaphore);
		void AddCommandBuffer(std::unique_ptr<VulkanCommandBuffer>&& pBuffer, vk::Semaphore waitSemaphore, std::vector<vk::PipelineStageFlags> waitStages);
		vk::Semaphore SubmitCommandBuffer(vk::Semaphore imageReadySemaphore);
	};
} // namespace Ailurus