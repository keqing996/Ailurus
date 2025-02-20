#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "Buffer.h"

namespace Ailurus
{
    class IndexBuffer : public Buffer
    {
    public:
        IndexBuffer(const Renderer* pRenderer, const std::vector<char>& indexData);
        ~IndexBuffer() override;

        vk::Buffer GetBuffer() const;

    private:
        vk::Buffer _buffer;
        vk::DeviceMemory _bufferMemory;
    };
}
