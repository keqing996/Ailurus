#pragma once

#include <queue>
#include "VulkanSystem/CommandBuffer/VulkanCommandBuffer.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class VulkanCommandBufferPool: public NonCopyable, public NonMovable
	{
	public:
		VulkanCommandBuffer Allocate();

		void Free(const VulkanCommandBuffer& commandBuffer);

		void Clear();

	private:
		std::queue<VulkanCommandBuffer> _availableCommandBuffers;
	};
}