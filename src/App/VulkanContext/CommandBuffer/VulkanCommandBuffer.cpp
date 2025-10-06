#include "VulkanCommandBuffer.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/Pipeline/VulkanPipeline.h"
#include "VulkanContext/DataBuffer/VulkanVertexBuffer.h"
#include "VulkanContext/DataBuffer/VulkanIndexBuffer.h"
#include "VulkanContext/Resource/DataBuffer/VulkanDataBuffer.h"
#include "VulkanContext/Resource/DataBuffer/VulkanDeviceBuffer.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"
#include "VulkanContext/Descriptor/VulkanDescriptorAllocator.h"

namespace Ailurus
{
	VulkanCommandBuffer::VulkanCommandBuffer(bool isPrimary)
		: _isPrimary(isPrimary)
	{
		vk::CommandBufferLevel level = _isPrimary
			? vk::CommandBufferLevel::ePrimary
			: vk::CommandBufferLevel::eSecondary;

		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.setCommandPool(VulkanContext::GetCommandPool())
			.setLevel(level)
			.setCommandBufferCount(1);

		try
		{
			const std::vector<vk::CommandBuffer> tempBuffers = VulkanContext::GetDevice().allocateCommandBuffers(allocInfo);
			_buffer = tempBuffers[0];
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to allocate command buffers: {}", e.what());
		}
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		try
		{
			_buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

			// Release resources
			ClearResourceReferences();

			// Recycle command buffer
			VulkanContext::GetDevice().freeCommandBuffers(VulkanContext::GetCommandPool(), _buffer);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to free command buffers: {}", e.what());
		}
	}

	const vk::CommandBuffer& VulkanCommandBuffer::GetBuffer() const
	{
		return _buffer;
	}

	bool VulkanCommandBuffer::IsRecording() const
	{
		return _isRecording;
	}

	void VulkanCommandBuffer::ClearResourceReferences()
	{
		for (auto& vulkanResource : _referencedResources)
			vulkanResource->RemoveRef(*this);

		_referencedResources.clear();
	}

	void VulkanCommandBuffer::Begin()
	{
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		if (!_isPrimary)
		{
			vk::CommandBufferInheritanceInfo inheritanceInfo;
			beginInfo.setPInheritanceInfo(&inheritanceInfo);
		}

		try
		{
			_buffer.begin(beginInfo);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to begin command buffer: {}", e.what());
		}

		_isRecording = true;
	}

	void VulkanCommandBuffer::End()
	{
		try
		{
			_buffer.end();
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to end command buffer: {}", e.what());
		}

		_isRecording = false;
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

	void VulkanCommandBuffer::ImageMemoryBarrier(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, 
		vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, 
		vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask)
	{
		vk::ImageMemoryBarrier barrier;
		barrier.setOldLayout(oldLayout)
			.setNewLayout(newLayout)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setImage(image)
			.setSubresourceRange(vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1))
			.setSrcAccessMask(srcAccessMask)
			.setDstAccessMask(dstAccessMask);

		_buffer.pipelineBarrier(srcStageMask, dstStageMask, {}, nullptr, nullptr, barrier);
	}

	void VulkanCommandBuffer::BeginRendering(vk::ImageView colorImageView, vk::ImageView depthImageView, vk::ImageView resolveImageView, vk::Extent2D extent, bool clearColor, bool useDepth)
	{
		// Color attachment
		vk::RenderingAttachmentInfo colorAttachment;
		colorAttachment.setImageView(colorImageView)
			.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setLoadOp(clearColor ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setClearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));

		// If resolve image is provided, set up MSAA resolve
		if (resolveImageView)
		{
			colorAttachment.setResolveMode(vk::ResolveModeFlagBits::eAverage)
				.setResolveImageView(resolveImageView)
				.setResolveImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
		}

		// Rendering info
		vk::RenderingInfo renderingInfo;
		renderingInfo.setRenderArea(vk::Rect2D{{0, 0}, extent})
			.setLayerCount(1)
			.setColorAttachments(colorAttachment);

		// Only add depth attachment if requested
		if (useDepth && depthImageView)
		{
			vk::RenderingAttachmentInfo depthAttachment;
			depthAttachment.setImageView(depthImageView)
				.setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setClearValue(vk::ClearDepthStencilValue(1.0f, 0));

			renderingInfo.setPDepthAttachment(&depthAttachment);
		}

		_buffer.beginRenderingKHR(renderingInfo);
	}

	void VulkanCommandBuffer::EndRendering()
	{
		_buffer.endRenderingKHR();
	}

	void VulkanCommandBuffer::SetViewportAndScissor()
	{
		const auto extent = VulkanContext::GetSwapChain()->GetConfig().extent;
		const vk::Viewport viewport(0.0f, 0.0f, extent.width, extent.height, 0.0f, 1.0f);
		const vk::Rect2D scissor(vk::Offset2D{ 0, 0 }, extent);
		_buffer.setViewport(0, 1, &viewport);
		_buffer.setScissor(0, 1, &scissor);
	}

	void VulkanCommandBuffer::BindPipeline(const VulkanPipeline* pPipeline)
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

	void VulkanCommandBuffer::PushConstantModelMatrix(const VulkanPipeline* pPipeline, const Matrix4x4f& modelMatrix)
	{
		if (pPipeline == nullptr)
		{
			Logger::LogError("VulkanCommandBuffer::PushConstantModelMatrix: Pipeline is nullptr");
			return;
		}

		// Change to column-major order for Vulkan
		auto columnMajorMatrix = modelMatrix.Transpose();

		// Record command
		_buffer.pushConstants(pPipeline->GetPipelineLayout(), 
			vk::ShaderStageFlagBits::eVertex, 
			0, 
			sizeof(Matrix4x4f), 
			&columnMajorMatrix);
	}

	void VulkanCommandBuffer::ExecuteSecondaryCommandBuffer(const VulkanCommandBuffer* pSecondaryCommandBuffer)
	{
		if (!_isPrimary)
		{
			Logger::LogError("Try execute secondary command buffer in a secondary command buffer!");
			return;
		}

		if (pSecondaryCommandBuffer == nullptr)
			return;

		_buffer.executeCommands(pSecondaryCommandBuffer->GetBuffer());
	}
} // namespace Ailurus