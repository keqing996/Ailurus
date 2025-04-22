#pragma once

#include "BufferType.h"
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class VertexBuffer
    {
    public:
        VertexBuffer(const void* vertexData, size_t dataSizeInBytes);
        ~VertexBuffer();

        vk::Buffer GetBuffer() const;
        size_t GetSize() const;

    private:
    	size_t _sizeInBytes;
        GpuBuffer _buffer;
    };
}
