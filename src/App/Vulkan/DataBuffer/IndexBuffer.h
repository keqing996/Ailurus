#pragma once

#include "Ailurus/Application/Assets/VertexAttributeDescription.h"
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	class IndexBuffer
	{
	public:
		IndexBuffer(IndexBufferFormat format, const void* indexData, size_t sizeInBytes);
		~IndexBuffer();

		vk::IndexType GetIndexType() const;
		vk::Buffer GetBuffer() const;
		size_t GetIndexCount() const;

	private:
		vk::IndexType _indexType;
		vk::Buffer _buffer;
		size_t _indexCount;
		vk::DeviceMemory _bufferMemory;
	};
} // namespace Ailurus
