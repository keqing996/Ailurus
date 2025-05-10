#pragma once

#include <cstdint>
#include <optional>
#include <vulkan/vulkan.hpp>
#include "VulkanSystem/CommandBuffer/VulkanCommandBuffer.h"

namespace Ailurus
{
	struct FrameContext
	{
		struct RenderingInfo
		{
			uint64_t renderingFrameCount;
			VulkanCommandBuffer renderingCommandBuffer;
		};

		vk::Semaphore imageReadySemaphore;
		vk::Semaphore renderFinishSemaphore;
		vk::Fence fence;

		// always available
		VulkanCommandBuffer availableCommandBuffer;

		// Is rendering target frame
		std::optional<RenderingInfo> renderingInfo;
		uint64_t lastRenderFinishedFrame;

    public:
		FrameContext();
		~FrameContext();

	public:
		bool IsRendering() const;
	};
} // namespace Ailurus