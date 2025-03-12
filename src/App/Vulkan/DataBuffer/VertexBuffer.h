#pragma once

#include <vulkan/vulkan.hpp>
#include "Buffer.h"

namespace Ailurus
{
    class VertexBuffer : public Buffer
    {
    public:
        VertexBuffer(const Render* pRenderer, const char* vertexData, size_t dataSizeInBytes);
        ~VertexBuffer() override;

        vk::Buffer GetBuffer() const;
        size_t GetSize() const;

    private:
        vk::Buffer _buffer;
        size_t _size;
        vk::DeviceMemory _bufferMemory;
    };
}
