#pragma once

#include <memory>
#include <unordered_set>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "VulkanSystem/Resource/VulkanBuffer.h"

namespace Ailurus
{
	class VulkanCommandBuffer : public NonCopyable
	{
	public:
		VulkanCommandBuffer();
		~VulkanCommandBuffer();

	public:
		const vk::CommandBuffer& GetBuffer() const;

		void CopyBuffer(VulkanHostBuffer& srcBuffer, VulkanDeviceBuffer& dstBuffer, vk::DeviceSize size);

	private:
		vk::CommandBuffer _buffer;
		std::unordered_set<VulkanResource*> _referencedResources;
	};

	struct VulkanCommandBufferRecordScope
	{
		VulkanCommandBufferRecordScope(const std::unique_ptr<VulkanCommandBuffer>& pCommandBuffer);
		~VulkanCommandBufferRecordScope();

	private:
		const vk::CommandBuffer _buffer;
	};
} // namespace Ailurus