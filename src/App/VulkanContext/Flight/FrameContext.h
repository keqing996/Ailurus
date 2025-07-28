#pragma once

#include <memory>
#include <optional>
#include "RenderingFrameContext.h"

namespace Ailurus
{
	struct FrameContext
	{
	public:
		~FrameContext();

	public:
		void EnsureFrameInitialized();
		bool WaitFinish();
		VulkanCommandBuffer* GetRecordingCommandBuffer() const;
		VulkanDescriptorAllocator* GetAllocatingDescriptorPool() const;
		vk::Semaphore SubmitCommandBuffer(vk::Semaphore imageReadySemaphore);

	private:
		// Current rendering (not fenced) frame context
		std::optional<RenderingFrameContext> _renderingFrameContext;

		// Recording buffer
		std::unique_ptr<VulkanCommandBuffer> _pRecordingCommandBuffer;

		// Allocating descriptor pool
		std::unique_ptr<VulkanDescriptorAllocator> _pAllocatingDescriptorPool;
	};
} // namespace Ailurus