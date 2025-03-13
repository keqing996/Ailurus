#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class VertexBuffer
    {
    public:
        VertexBuffer(const char* vertexData, size_t dataSizeInBytes);
        ~VertexBuffer();

        vk::Buffer GetBuffer() const;
        size_t GetSize() const;

    private:
        vk::Buffer _buffer;
        size_t _size;
        vk::DeviceMemory _bufferMemory;
    };
}
