#pragma once

#include <cstdint>
#include <optional>
#include <vulkan/vulkan.hpp>
#include "VulkanSystem/CommandBuffer/VulkanCommandBuffer.h"

namespace Ailurus
{
	class FrameContext
	{
    public:
		struct RenderingInfo
		{
			uint64_t renderingFrameCount;
			VulkanCommandBuffer renderingCommandBuffer;
			std::vector<vk::Semaphore> usingSemaphores;
			vk::Fence allFinishFence;
		};

		FrameContext();
		~FrameContext();

	public:
		bool IsRendering() const;
		const VulkanCommandBuffer& GetCommandBuffer();
		uint64_t LastRenderFinishedFrame() const;

	private:
		std::optional<RenderingInfo> renderingInfo;
		uint64_t lastRenderFinishedFrame;

		std::vector<vk::Semaphore> _unSubmittedSemaphores;
		std::optional<VulkanCommandBuffer> _frameCommandBuffer;
	};
} // namespace Ailurus