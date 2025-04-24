#pragma once

#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class FrameResource : public NonCopyable, public NonMovable
	{
	public:
		FrameResource();
		~FrameResource();

	public:
		vk::Semaphore GetSemaphoreImageAvailable() const;
		vk::Semaphore GetSemaphoreRenderFinished() const;
		vk::Fence GetInFlightFence() const;
		vk::CommandBuffer GetCommandBuffer() const;

	private:
		vk::CommandPool _commandPool;
		vk::Semaphore _imageAvailableSemaphore;
		vk::Semaphore _renderFinishedSemaphore;
		vk::Fence _inFlightFence;
		vk::CommandBuffer _commandBuffer;
	};
} // namespace Ailurus