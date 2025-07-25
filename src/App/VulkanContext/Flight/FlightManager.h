#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
	class FlightManager : public NonCopyable, public NonMovable
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

	public:
		FlightManager();
		~FlightManager();

	private:
	};
} // namespace Ailurus