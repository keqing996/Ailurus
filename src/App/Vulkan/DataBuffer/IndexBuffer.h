#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class IndexBuffer
    {
    public:
        explicit IndexBuffer(std::vector<uint16_t> indexData);
        explicit IndexBuffer(std::vector<uint32_t> indexData);
        ~IndexBuffer();

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
