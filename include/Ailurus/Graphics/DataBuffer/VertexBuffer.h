#pragma once

#include <vulkan/vulkan.hpp>
#include "Buffer.h"

namespace Ailurus
{
    class VertexBuffer : public Buffer
    {
    public:
        VertexBuffer(const Renderer* pRenderer, const char* vertexData, size_t vertexSize);
        ~VertexBuffer() override;

        vk::Buffer GetBuffer() const;

    private:
        vk::Buffer _buffer;
        vk::DeviceMemory _bufferMemory;
    };
}
