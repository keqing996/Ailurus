#pragma once

#include "VulkanContext/VulkanPch.h"

namespace Ailurus
{
    class VulkanVertexBuffer
    {
    public:
        VulkanVertexBuffer(const void* vertexData, size_t dataSizeInBytes);
        ~VulkanVertexBuffer();

        class VulkanDeviceBuffer* GetBuffer() const;
        size_t GetSize() const;

    private:
    	size_t _sizeInBytes;
        class VulkanDeviceBuffer* _buffer;
    };
}
