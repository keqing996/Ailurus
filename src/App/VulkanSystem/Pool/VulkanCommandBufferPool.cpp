#include "VulkanCommandBufferPool.h"

namespace Ailurus
{
	VulkanCommandBuffer VulkanCommandBufferPool::Allocate()
	{
		if (_availableCommandBuffers.empty())
			return {};

		auto ret = _availableCommandBuffers.front();
		_availableCommandBuffers.pop();
		return ret;
	}

	void VulkanCommandBufferPool::Free(const VulkanCommandBuffer& commandBuffer)
	{
		_availableCommandBuffers.push(commandBuffer);
	}

	void VulkanCommandBufferPool::Clear()
	{
		while (!_availableCommandBuffers.empty())
			_availableCommandBuffers.pop();
	}
} // namespace Ailurus
