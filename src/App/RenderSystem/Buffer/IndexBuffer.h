#pragma once

#include "Ailurus/Application/AssetsSystem//VertexAttributeDescription.h"
#include "BufferType.h"
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
		size_t _indexCount;
		size_t _sizeInBytes;
		GpuBuffer _buffer;
	};
} // namespace Ailurus
