#include "VulkanIndexBuffer.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/Helper/VulkanHelper.h"
#include "VulkanSystem/VulkanSystem.h"
#include "VulkanSystem/Resource/VulkanResourceManager.h"
#include "VulkanSystem/CommandBuffer/VulkanCommandBuffer.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	VulkanIndexBuffer::VulkanIndexBuffer(IndexBufferFormat format, const void* indexData, size_t sizeInBytes)
		: _sizeInBytes(sizeInBytes)
	{
		switch (format)
		{
			case IndexBufferFormat::UInt16:
				_indexType = vk::IndexType::eUint16;
				break;
			case IndexBufferFormat::UInt32:
				_indexType = vk::IndexType::eUint32;
				break;
			default:
				Logger::LogError("Index buffer type {} does not have related vulkan type",
						EnumReflection<IndexBufferFormat>::ToString(format));
				return;
		}

		const auto indexSize = VulkanHelper::SizeOf(format);
		_indexCount = sizeInBytes / indexSize;
		if (sizeInBytes % indexSize != 0)
		{
			Logger::LogError("Index buffer data size is not a multiple of {}",
				EnumReflection<IndexBufferFormat>::ToString(format));
		}

		const auto pVulkanResManager = Application::Get<VulkanSystem>()->GetResourceManager();

		// Create gpu buffer
		_buffer = pVulkanResManager->CreateDeviceBuffer(sizeInBytes, DeviceBufferUsage::Index);
    	if (_buffer == nullptr)
    		return;

		// Create cpu stage buffer
		auto stageBuffer = pVulkanResManager->CreateHostBuffer(sizeInBytes, HostBufferUsage::TransferSrc);
    	if (stageBuffer == nullptr)
			return;

		// Cpu -> Cpu buffer
		std::memcpy(stageBuffer->mappedAddr, indexData, sizeInBytes);

		// Cpu buffer -> Gpu buffer
		VulkanCommandBuffer* pCommandBuffer = Application::Get<VulkanSystem>()->GetFrameContext()->GetRecordingCommandBuffer();
		pCommandBuffer->CopyBuffer(stageBuffer, _buffer, sizeInBytes);
		pCommandBuffer->BufferMemoryBarrier(_buffer, vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eIndexRead, 
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eVertexInput);

    	// Destroy stage cpu buffer
    	stageBuffer->MarkDelete();
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		_buffer->MarkDelete();
	}

	vk::IndexType VulkanIndexBuffer::GetIndexType() const
	{
		return _indexType;
	}

	VulkanDeviceBuffer* VulkanIndexBuffer::GetBuffer() const
	{
		return _buffer;
	}

	size_t VulkanIndexBuffer::GetIndexCount() const
	{
		return _indexCount;
	}
} // namespace Ailurus
