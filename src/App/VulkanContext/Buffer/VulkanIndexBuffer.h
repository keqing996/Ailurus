#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Application/RenderSystem/Vertex/IndexBufferFormat.h>

namespace Ailurus
{
	class VulkanIndexBuffer
	{
	public:
		VulkanIndexBuffer(IndexBufferFormat format, const void* indexData, size_t sizeInBytes);
		~VulkanIndexBuffer();

		vk::IndexType GetIndexType() const;
		class VulkanDeviceBuffer* GetBuffer() const;
		size_t GetIndexCount() const;

	private:
		vk::IndexType _indexType;
		size_t _indexCount;
		size_t _sizeInBytes;
		VulkanDeviceBuffer* _buffer;
	};
} // namespace Ailurus
