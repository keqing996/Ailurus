#include "VulkanCommandBuffer.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"

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

	void VulkanCommandBuffer::CopyBuffer(VulkanHostBuffer& srcBuffer, VulkanDeviceBuffer& dstBuffer, vk::DeviceSize size)
	{
        // Record resources
        srcBuffer.AddRef(*this);
        _referencedResources.insert(&srcBuffer);

        dstBuffer.AddRef(*this);
        _referencedResources.insert(&dstBuffer);

        // Record command
		vk::BufferCopy copyRegion;
		copyRegion.setSize(size)
			.setSrcOffset(0)
			.setDstOffset(0);

		_buffer.copyBuffer(srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);
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
} // namespace Ailurus