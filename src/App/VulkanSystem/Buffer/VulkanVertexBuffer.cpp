#include "VulkanVertexBuffer.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"
#include "VulkanSystem/Resource/VulkanResourceManager.h"

namespace Ailurus
{
    VulkanVertexBuffer::VulkanVertexBuffer(const void* vertexData, size_t sizeInBytes)
        : _sizeInBytes(sizeInBytes)
    {
		auto pVulkanResManager = Application::Get<VulkanSystem>()->GetResourceManager();

    	// Create gpu buffer
		_buffer = pVulkanResManager->CreateDeviceBuffer(sizeInBytes, DeviceBufferUsage::Vertex);
    	if (_buffer == nullptr)
    		return;

    	// Create cpu stage buffer
		auto stageBuffer = pVulkanResManager->CreateHostBuffer(sizeInBytes, HostBufferUsage::TransferSrc);
    	if (stageBuffer == nullptr)
			return;

    	// Cpu -> Cpu buffer
    	std::memcpy(stageBuffer->mappedAddr, vertexData, sizeInBytes);

    	// Cpu buffer -> Gpu buffer
    	std::unique_ptr<VulkanCommandBuffer> pCommandBuffer = std::make_unique<VulkanCommandBuffer>();
		{
			VulkanCommandBufferRecordScope recordScope(pCommandBuffer);
			pCommandBuffer->CopyBuffer(stageBuffer, _buffer, sizeInBytes);
		}
		Application::Get<VulkanSystem>()->AddCommandBuffer(std::move(pCommandBuffer));

    	// Destroy stage cpu buffer
    	stageBuffer->MarkDelete();
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
    	_buffer->MarkDelete();
    }

    vk::Buffer VulkanVertexBuffer::GetBuffer() const
    {
        return _buffer->buffer;
    }

    size_t VulkanVertexBuffer::GetSize() const
    {
        return _sizeInBytes;
    }
}
