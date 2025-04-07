#include "IndexBuffer.h"
#include "DataBufferUtil.h"
#include "Vulkan/Context/VulkanContext.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	IndexBuffer::IndexBuffer(IndexBufferFormat format, const void* indexData, size_t sizeInBytes)
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

		auto ret = DataBufferUtil::CreateBuffer(BufferType::Index, indexData, sizeInBytes);
		if (!ret)
			return;

		_buffer = ret->buffer;
		_bufferMemory = ret->deviceMemory;
	}

	IndexBuffer::~IndexBuffer()
	{
		DataBufferUtil::DestroyBuffer(_buffer, _bufferMemory);
	}

	vk::IndexType IndexBuffer::GetIndexType() const
	{
		return _indexType;
	}

	vk::Buffer IndexBuffer::GetBuffer() const
	{
		return _buffer;
	}

	size_t IndexBuffer::GetIndexCount() const
	{
		return _indexCount;
	}
} // namespace Ailurus
