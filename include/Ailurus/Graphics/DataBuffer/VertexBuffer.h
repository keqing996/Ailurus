#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "Buffer.h"

namespace Ailurus
{
    class VertexBuffer : public Buffer
    {
    public:
        VertexBuffer(const Renderer* pRenderer, const std::vector<char>& vertexData);
        ~VertexBuffer() override;

        vk::Buffer GetBuffer() const;

    private:
        vk::Buffer _buffer;
        vk::DeviceMemory _bufferMemory;
    };
}
