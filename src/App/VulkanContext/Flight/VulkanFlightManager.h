#pragma once

#include <deque>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
	class VulkanCommandBuffer;
	class VulkanDescriptorAllocator;

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

	public:
		VulkanFlightManager(uint32_t parallelFrames);
		~VulkanFlightManager();

	public:
		void EnsurePreparation();
		bool WaitOneFlight();
		void WaitAllFlight();
		bool TakeOffFlight(uint32_t imageIndex, vk::Semaphore imageReadySemaphore, bool* needRebuild);
		auto GetRecordingCommandBuffer() -> VulkanCommandBuffer*;
		auto GetAllocatingDescriptorPool() -> VulkanDescriptorAllocator*;

	private:
		const uint32_t _parallelFrame;

		// Current rendering (not fenced) frame context
		std::deque<OnAirFrame> _onAirFrames;

		// Recording buffer & descriptor allocator
		std::unique_ptr<VulkanCommandBuffer> _pRecordingCommandBuffer;
		std::unique_ptr<VulkanDescriptorAllocator> _pAllocatingDescriptorPool;
	};
} // namespace Ailurus