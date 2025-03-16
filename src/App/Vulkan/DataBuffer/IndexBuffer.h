#pragma once

#include "Ailurus/Application/Render/InputAttribute.h"
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class IndexBuffer
    {
    public:
        IndexBuffer(IndexBufferFormat format, const char* indexData, size_t sizeInBytes);
        ~IndexBuffer();

        vk::IndexType GetIndexType() const;
        vk::Buffer GetBuffer() const;
        size_t GetIndexCount() const;

    private:
        vk::IndexType _indexType;
        vk::Buffer _buffer;
        size_t _indexCount;
        vk::DeviceMemory _bufferMemory;
    };
}
