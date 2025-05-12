#include "VulkanIndexBuffer.h"
#include "DataBufferUtil.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	VulkanIndexBuffer::VulkanIndexBuffer(IndexBufferFormat format, const void* indexData, size_t sizeInBytes)
		: _sizeInBytes(sizeInBytes)
	{
		switch (format)
		{
			case IndexBufferFormat::UInt16:
			{
				_indexType = vk::IndexType::eUint16;
				_indexCount = sizeInBytes / sizeof(uint16_t);
				if (sizeInBytes % sizeof(uint16_t) != 0)
					Logger::LogError("Index buffer data size is not a multiple of uint16_t");
				break;
			}
			case IndexBufferFormat::UInt32:
			{
				_indexType = vk::IndexType::eUint32;
				_indexCount = sizeInBytes / sizeof(uint32_t);
				if (sizeInBytes % sizeof(uint32_t) != 0)
					Logger::LogError("Index buffer data size is not a multiple of uint32_t");
				break;
			}
			default:
			{
				Logger::LogError("Index buffer type {} does not have related vulkan type",
					EnumReflection<IndexBufferFormat>::ToString(format));
				return;
			}
		}

		// Create gpu buffer
		const auto retCreateGpuBuffer = DataBufferUtil::CreateGpuBuffer(sizeInBytes, GpuBufferUsage::Index);
		if (!retCreateGpuBuffer.has_value())
			return;

		_buffer = *retCreateGpuBuffer;

		// Create cpu stage buffer
		const auto retCreateStageBuffer = DataBufferUtil::CreateCpuBuffer(sizeInBytes, CpuBufferUsage::TransferSrc);
		if (!retCreateStageBuffer.has_value())
			return;

		CpuBuffer stageBuffer = *retCreateStageBuffer;

		// Cpu -> Cpu buffer
		std::memcpy(stageBuffer.mappedAddr, indexData, sizeInBytes);

		// Cpu buffer -> Gpu buffer
		std::unique_ptr<VulkanCommandBuffer> pCommandBuffer = std::make_unique<VulkanCommandBuffer>();
		{
			VulkanCommandBufferRecordScope recordScope(pCommandBuffer);
			pCommandBuffer->CopyBuffer(srcBuffer, dstBuffer, size);
		}
		Application::Get<VulkanSystem>()->AddCommandBuffer(std::move(pCommandBuffer));

		// Destroy stage cpu buffer
		DataBufferUtil::DestroyBuffer(stageBuffer);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		DataBufferUtil::DestroyBuffer(_buffer);
	}

	vk::IndexType VulkanIndexBuffer::GetIndexType() const
	{
		return _indexType;
	}

	vk::Buffer VulkanIndexBuffer::GetBuffer() const
	{
		return _buffer.buffer;
	}

	size_t VulkanIndexBuffer::GetIndexCount() const
	{
		return _indexCount;
	}
} // namespace Ailurus
