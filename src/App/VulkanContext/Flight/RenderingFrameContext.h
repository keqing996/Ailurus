#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class VulkanCommandBuffer;
	class VulkanDescriptorAllocator;
    
	struct RenderingFrameContext
	{
		uint64_t renderingFrameCount;
		std::unique_ptr<VulkanCommandBuffer> pRenderingCommandBuffer;
		std::unique_ptr<VulkanDescriptorAllocator> pFrameDescriptorPool;
		vk::Semaphore waitSemaphore;
		vk::Semaphore signalSemaphore;
		vk::Fence allFinishFence;
	};
} // namespace Ailurus