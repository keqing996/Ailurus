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

	void VulkanCommandBuffer::CopyBuffer(VulkanHostBuffer* pSrcBuffer, VulkanDeviceBuffer* pDstBuffer, vk::DeviceSize size)
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

	VulkanCommandBufferRecordScope::VulkanCommandBufferRecordScope(const std::unique_ptr<VulkanCommandBuffer>& pCommandBuffer)
		: _buffer(pCommandBuffer->GetBuffer())
	{
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		_buffer.begin(beginInfo);
	}

	VulkanCommandBufferRecordScope::~VulkanCommandBufferRecordScope()
	{
		_buffer.end();
	}

	VulkanCommandBufferRenderPassRecordScope::VulkanCommandBufferRenderPassRecordScope(const std::unique_ptr<VulkanCommandBuffer>& pCommandBuffer, const RenderPass* pRenderPass)
		: _buffer(pCommandBuffer->GetBuffer())
	{
		_buffer.beginRenderPass(pRenderPass->GetRHIRenderPass()->GetRenderPassBeginInfo(), {});
	}

	VulkanCommandBufferRenderPassRecordScope::~VulkanCommandBufferRenderPassRecordScope()
	{
		_buffer.endRenderPass();
	}
} // namespace Ailurus