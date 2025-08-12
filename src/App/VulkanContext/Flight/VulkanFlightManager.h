#pragma once

#include <deque>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"
#include "VulkanContext/Descriptor/VulkanDescriptorAllocator.h"

namespace Ailurus
{
	class VulkanFlightManager : public NonCopyable, public NonMovable
	{
		struct OnAirFrame
		{
			uint64_t frameCount;
			std::unique_ptr<VulkanCommandBuffer> pRenderingCommandBuffer;
			std::unique_ptr<VulkanDescriptorAllocator> pFrameDescriptorAllocator;
			vk::Semaphore imageReadySemaphore;
			vk::Semaphore renderFinishSemaphore;
			vk::Fence renderFinishFence;
		};

	public:
		explicit VulkanFlightManager(uint32_t parallelFrames);
		~VulkanFlightManager();

	public:
		void EnsurePreparation();
		bool WaitOneFlight();
		void WaitAllFlight();
		bool TakeOffFlight(uint32_t imageIndex, vk::Semaphore imageReadySemaphore, bool* needRebuild);
		auto GetRecordingCommandBuffer() -> VulkanCommandBuffer*;
		auto GetAllocatingDescriptorPool() -> VulkanDescriptorAllocator*;
		uint32_t GetParallelFramesCount() const;

	private:
		const uint32_t _parallelFrame;

		// Current rendering (not fenced) frame context
		std::deque<OnAirFrame> _onAirFrames;

		// Recording buffer & descriptor allocator
		std::unique_ptr<VulkanCommandBuffer> _pRecordingCommandBuffer;
		std::unique_ptr<VulkanDescriptorAllocator> _pAllocatingDescriptorPool;
	};

} // namespace Ailurus