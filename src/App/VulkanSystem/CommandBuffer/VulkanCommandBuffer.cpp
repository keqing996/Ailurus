#include "VulkanCommandBuffer.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPass.h"
#include "VulkanSystem/RenderPass/VulkanRenderPass.h"

namespace Ailurus
{

	VulkanCommandBuffer::VulkanCommandBuffer()
		: _buffer(Application::Get<VulkanSystem>()->AllocateCommandBuffer())
	{
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		// Release resources
		for (auto& vulkanResource : _referencedResources)
			vulkanResource->RemoveRef(*this);

		// Recycle command buffer
		Application::Get<VulkanSystem>()->FreeCommandBuffer(_buffer);
	}

	const vk::CommandBuffer& VulkanCommandBuffer::GetBuffer() const
	{
		return _buffer;
	}

	void VulkanCommandBuffer::Begin()
	{
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		_buffer.begin(beginInfo);
	}

	void VulkanCommandBuffer::End()
	{
		_buffer.end();
	}

	void VulkanCommandBuffer::CopyBuffer(VulkanDataBuffer* pSrcBuffer, VulkanDataBuffer* pDstBuffer, vk::DeviceSize size)
	{
		if (pSrcBuffer == nullptr || pDstBuffer == nullptr)
		{
			Logger::LogError("VulkanCommandBuffer::CopyBuffer: Source or destination buffer is nullptr");
			return;
		}

		// Record resources
		pSrcBuffer->AddRef(*this);
		_referencedResources.insert(pSrcBuffer);

		pDstBuffer->AddRef(*this);
		_referencedResources.insert(pDstBuffer);

		// Record command
		vk::BufferCopy copyRegion;
		copyRegion.setSize(size)
			.setSrcOffset(0)
			.setDstOffset(0);

		_buffer.copyBuffer(pSrcBuffer->buffer, pDstBuffer->buffer, 1, &copyRegion);
	}

	void VulkanCommandBuffer::BufferMemoryBarrier(VulkanDataBuffer* pBuffer, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask)
	{
		if (pBuffer == nullptr)
			return;

		// Record resources
		pBuffer->AddRef(*this);
		_referencedResources.insert(pBuffer);

		// Record command
		vk::BufferMemoryBarrier barrier;
		barrier.setBuffer(pBuffer->buffer)
			.setSrcAccessMask(srcAccessMask)
			.setDstAccessMask(dstAccessMask)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setSize(VK_WHOLE_SIZE);

		_buffer.pipelineBarrier(srcStageMask, dstStageMask, {}, nullptr, barrier, nullptr);
	}

	VulkanCommandBufferRenderPassRecordScope::VulkanCommandBufferRenderPassRecordScope(const VulkanCommandBuffer* pCommandBuffer, const RenderPass* pRenderPass)
		: _buffer(pCommandBuffer->GetBuffer())
	{
		_buffer.beginRenderPass(pRenderPass->GetRHIRenderPass()->GetRenderPassBeginInfo(), {});
	}

	VulkanCommandBufferRenderPassRecordScope::~VulkanCommandBufferRenderPassRecordScope()
	{
		_buffer.endRenderPass();
	}
} // namespace Ailurus