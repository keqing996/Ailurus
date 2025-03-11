#pragma once

#include <vulkan/vulkan.hpp>
#include "Buffer.h"

namespace Ailurus
{
    class IndexBuffer : public Buffer
    {
    public:
        IndexBuffer(const Render* pRenderer, std::vector<uint16_t> indexData);
        IndexBuffer(const Render* pRenderer, std::vector<uint32_t> indexData);
        ~IndexBuffer() override;

        vk::IndexType GetIndexType() const;
        vk::Buffer GetBuffer() const;
        size_t GetIndexCount() const;

    private:
        void InternalCreate(const char* dataBuffer, size_t dataSizeInBytes);

    private:
        vk::IndexType _indexType;
        vk::Buffer _buffer;
        size_t _indexCount;
        vk::DeviceMemory _bufferMemory;
    };
}
