#pragma once

#include <vulkan/vulkan.hpp>
#include "Buffer.h"

namespace Ailurus
{
    class IndexBuffer : public Buffer
    {
    public:
        IndexBuffer(const Renderer* pRenderer, const char* indexData, size_t indexSize);
        ~IndexBuffer() override;

        vk::Buffer GetBuffer() const;

    private:
        vk::Buffer _buffer;
        vk::DeviceMemory _bufferMemory;
    };
}
