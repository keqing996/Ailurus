#pragma once

#include <deque>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "FrameContext.h"

namespace Ailurus
{
	class VulkanFlightManager : public NonCopyable, public NonMovable
	{
		struct OnAirFrame
		{
			uint64_t frameCount;
			std::unique_ptr<VulkanCommandBuffer> pRenderingCommandBuffer;
			std::unique_ptr<VulkanDescriptorAllocator> pFrameDescriptorAllocator;
			vk::Semaphore waitSemaphore;
			vk::Semaphore signalSemaphore;
			vk::Fence allFinishFence;
		};

		struct PrepareFrame
		{
			std::unique_ptr<VulkanCommandBuffer> pRecordingCommandBuffer;
			std::unique_ptr<VulkanDescriptorAllocator> pAllocatingDescriptorPool;
		};

	public:
		VulkanFlightManager(uint32_t parallelFrames);
		~VulkanFlightManager();

	public:
		bool WaitOneFlight();
		void WaitAllFlight();
		bool TakeOffFlight(uint32_t imageIndex, vk::Semaphore imageReadySemaphore, bool* needRebuild);

	private:
		FrameContext* GetFrameContext();

	private:
		const uint32_t _parallelFrame;
		uint32_t _currentParallelFrameIndex = 0;
		std::vector<std::unique_ptr<FrameContext>> _frameContexts{};

		std::deque<OnAirFrame> _onAirFrames;
		PrepareFrame _preparingFrame;
	};
} // namespace Ailurus