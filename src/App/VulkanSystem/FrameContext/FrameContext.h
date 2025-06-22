#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vulkan/vulkan.hpp>
#include <VulkanSystem/CommandBuffer/VulkanCommandBuffer.h>
#include <VulkanSystem/Descriptor/VulkanDescriptorPool.h>

namespace Ailurus
{
	struct RenderingFrameContext
	{
		uint64_t renderingFrameCount;
		std::unique_ptr<VulkanCommandBuffer> pRenderingCommandBuffer;
		std::unique_ptr<VulkanDescriptorPool> pFrameDescriptorPool;
		vk::Semaphore waitSemaphore;
		vk::Semaphore signalSemaphore;
		vk::Fence allFinishFence;
	};

	struct FrameContext
	{
	public:
		~FrameContext();

	public:
		void EnsureFrameInitialized();
		bool WaitFinish();
		VulkanCommandBuffer* GetRecordingCommandBuffer() const;
		VulkanDescriptorPool* GetAllocatingDescriptorPool() const;
		vk::Semaphore SubmitCommandBuffer(vk::Semaphore imageReadySemaphore);

	private:
		// Current rendering (not fenced) frame context
		std::optional<RenderingFrameContext> _renderingFrameContext;

		// Recording buffer
		std::unique_ptr<VulkanCommandBuffer> _pRecordingCommandBuffer;

		// Allocating descriptor pool
		std::unique_ptr<VulkanDescriptorPool> _pAllocatingDescriptorPool;
	};
} // namespace Ailurus