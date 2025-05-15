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

		void Begin();
		void End();
		void CopyBuffer(VulkanDataBuffer* pSrcBuffer, VulkanDataBuffer* pDstBuffer, vk::DeviceSize size);
		void BufferMemoryBarrier(VulkanDataBuffer* pBuffer, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask);

	private:
		vk::CommandBuffer _buffer;
		std::unordered_set<VulkanResource*> _referencedResources;
	};

	struct VulkanCommandBufferRenderPassRecordScope
	{
		VulkanCommandBufferRenderPassRecordScope(const VulkanCommandBuffer* pCommandBuffer, const class RenderPass* pRenderPass);
		~VulkanCommandBufferRenderPassRecordScope();

	private:
		const vk::CommandBuffer _buffer;
	};
} // namespace Ailurus