#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vulkan/vulkan.hpp>
#include "VulkanSystem/CommandBuffer/VulkanCommandBuffer.h"

namespace Ailurus
{
	struct RenderingFrameContext
	{
		uint64_t renderingFrameCount;
		std::unique_ptr<VulkanCommandBuffer> pRenderingCommandBuffer;
		vk::Semaphore waitSemaphore;
		vk::Semaphore signalSemaphore;
		vk::Fence allFinishFence;
	};

	struct FrameContext
	{
	public:
		void EnsureCommandBufferExist();
		bool WaitFinish();
		VulkanCommandBuffer* GetRecordingCommandBuffer() const;
		vk::Semaphore SubmitCommandBuffer(vk::Semaphore imageReadySemaphore);

	private:
		// Current rendering (not fenced) frame context
		std::optional<RenderingFrameContext> _renderingFrameContext;

		// Recording buffer
		std::unique_ptr<VulkanCommandBuffer> _pRecordingCommandBuffer;
	};
} // namespace Ailurus