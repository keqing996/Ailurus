#include "VulkanCommandBuffer.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPass.h"
#include "VulkanSystem/RenderPass/VulkanRenderPass.h"
#include "VulkanSystem/Pipeline/VulkanPipeline.h"
#include "VulkanSystem/Buffer/VulkanVertexBuffer.h"
#include "VulkanSystem/Buffer/VulkanIndexBuffer.h"

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

	void VulkanCommandBuffer::BeginRenderPass(VulkanRenderPass* pRenderPass)
	{
		if (pRenderPass == nullptr)
			return;

		_buffer.beginRenderPass(pRenderPass->GetRenderPassBeginInfo(), {});
	}

	void VulkanCommandBuffer::EndRenderPass()
	{
		_buffer.endRenderPass();
	}

	void VulkanCommandBuffer::SetViewportAndScissor()
	{
		const auto extent = Application::Get<VulkanSystem>()->GetSwapChainConfig().extent;
		const vk::Viewport viewport(0.0f, 0.0f, extent.width, extent.height, 0.0f, 1.0f);
		const vk::Rect2D scissor(vk::Offset2D{ 0, 0 }, extent);
		_buffer.setViewport(0, 1, &viewport);
		_buffer.setScissor(0, 1, &scissor);
	}

	void VulkanCommandBuffer::BindPipeline(const class VulkanPipeline* pPipeline)
	{
		if (pPipeline == nullptr)
			return;

		_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pPipeline->GetPipeline());
	}

	void VulkanCommandBuffer::BindVertexBuffer(const VulkanVertexBuffer* pVertexBuffer)
	{
		if (pVertexBuffer == nullptr)
		{
			Logger::LogError("VulkanCommandBuffer::BindVertexBuffer: Vertex buffer is nullptr");
			return;
		}

		// Record resources
		auto* pInternalBuffer = pVertexBuffer->GetBuffer();
		pInternalBuffer->AddRef(*this);
		_referencedResources.insert(pInternalBuffer);

		// Record command
		const vk::DeviceSize offset = 0;
		_buffer.bindVertexBuffers(0, 1, &pInternalBuffer->buffer, &offset);
	}

	void VulkanCommandBuffer::BindIndexBuffer(const VulkanIndexBuffer* pIndexBuffer)
	{
		if (pIndexBuffer == nullptr)
		{
			Logger::LogError("VulkanCommandBuffer::BindIndexBuffer: Index buffer is nullptr");
			return;
		}

		// Record resources
		auto* pInternalBuffer = pIndexBuffer->GetBuffer();
		pInternalBuffer->AddRef(*this);
		_referencedResources.insert(pInternalBuffer);

		// Record command
		const vk::DeviceSize offset = 0;
		_buffer.bindIndexBuffer(pInternalBuffer->buffer, offset, pIndexBuffer->GetIndexType());
	}

	void VulkanCommandBuffer::BindDescriptorSet(vk::PipelineLayout layout, const std::vector<vk::DescriptorSet>& descriptorSets)
	{
		_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, descriptorSets, nullptr);
	}

	void VulkanCommandBuffer::DrawIndexed(uint32_t indexCount)
	{
		_buffer.drawIndexed(indexCount, 1, 0, 0, 0);
	}

	void VulkanCommandBuffer::DrawNonIndexed(uint32_t vertexCount)
	{
		_buffer.draw(vertexCount, 1, 0, 0);
	}

	void VulkanCommandBuffer::PushConstantModelMaterix(const VulkanPipeline* pPipeline, const Matrix4x4f& modelMatrix)
	{
		if (pPipeline == nullptr)
		{
			Logger::LogError("VulkanCommandBuffer::PushConstantModelMaterix: Pipeline is nullptr");
			return;
		}

		// Record command
		_buffer.pushConstants(pPipeline->GetPipelineLayout(), 
			vk::ShaderStageFlagBits::eVertex, 
			0, 
			sizeof(Matrix4x4f), 
			&modelMatrix);
	}
} // namespace Ailurus