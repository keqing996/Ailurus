#pragma once

#include "Ailurus/Application/AssetsSystem/VertexAttributeDescription.h"
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	class VulkanIndexBuffer
	{
	public:
		VulkanIndexBuffer(IndexBufferFormat format, const void* indexData, size_t sizeInBytes);
		~VulkanIndexBuffer();

		vk::IndexType GetIndexType() const;
		vk::Buffer GetBuffer() const;
		size_t GetIndexCount() const;

	private:
		vk::IndexType _indexType;
		size_t _indexCount;
		size_t _sizeInBytes;
		class VulkanDeviceBuffer* _buffer;
	};
} // namespace Ailurus
