#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class VulkanVertexBuffer
    {
    public:
        VulkanVertexBuffer(const void* vertexData, size_t dataSizeInBytes);
        ~VulkanVertexBuffer();

        vk::Buffer GetBuffer() const;
        size_t GetSize() const;

    private:
    	size_t _sizeInBytes;
        class VulkanDeviceBuffer* _buffer;
    };
}
