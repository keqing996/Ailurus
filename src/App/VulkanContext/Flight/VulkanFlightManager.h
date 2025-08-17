#pragma once

#include <vector>
#include <optional>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"
#include "VulkanContext/Descriptor/VulkanDescriptorAllocator.h"
#include "VulkanContext/Fence/VulkanFence.h"
#include "VulkanContext/Semaphore/VulkanSemaphore.h"

namespace Ailurus
{
	class VulkanFlightManager : public NonCopyable, public NonMovable
	{
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
		struct OnAirInfo
		{
			uint64_t frameCount;
		};
		
		struct FrameContext
		{
			std::optional<OnAirInfo> onAirInfo;	
			std::unique_ptr<VulkanCommandBuffer> pRenderingCommandBuffer;
			std::unique_ptr<VulkanDescriptorAllocator> pFrameDescriptorAllocator;
			std::unique_ptr<VulkanSemaphore> imageReadySemaphore;
			std::unique_ptr<VulkanSemaphore> renderFinishSemaphore;
			std::unique_ptr<VulkanFence> renderFinishFence;
		};

	private:
		const uint32_t _parallelFrame;

		uint32_t _currentFlightIndex = 0;
		std::vector<FrameContext> _frameContext;
	};

} // namespace Ailurus