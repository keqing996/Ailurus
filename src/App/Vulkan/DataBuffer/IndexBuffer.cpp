#include "IndexBuffer.h"
#include "DataBufferUtil.h"
#include "Vulkan/Context/VulkanContext.h"

namespace Ailurus
{
    IndexBuffer::IndexBuffer(std::vector<uint16_t> indexData)
        : _indexType(vk::IndexType::eUint16)
        , _buffer(nullptr)
        , _indexCount(indexData.size())
        , _bufferMemory(nullptr)
    {
        InternalCreate(reinterpret_cast<const char*>(indexData.data()),
            indexData.size() * sizeof(uint16_t));
    }

    IndexBuffer::IndexBuffer(std::vector<uint32_t> indexData)
        : _indexType(vk::IndexType::eUint32)
        , _buffer(nullptr)
        , _indexCount(indexData.size())
        , _bufferMemory(nullptr)
    {
        InternalCreate(reinterpret_cast<const char*>(indexData.data()),
                    indexData.size() * sizeof(uint32_t));
    }

    void IndexBuffer::InternalCreate(const char* dataBuffer, size_t dataSizeInBytes)
    {
        auto ret = DataBufferUtil::CreateBuffer(BufferType::Index, dataBuffer, dataSizeInBytes);
        if (!ret)
            return;

        _buffer = ret->buffer;
        _bufferMemory = ret->deviceMemory;
    }

    IndexBuffer::~IndexBuffer()
    {
        const auto device = VulkanContext::GetDevice();
        device.freeMemory(_bufferMemory);
        device.destroyBuffer(_buffer);
    }

    vk::IndexType IndexBuffer::GetIndexType() const
    {
        return _indexType;
    }

    vk::Buffer IndexBuffer::GetBuffer() const
    {
        return _buffer;
    }

    size_t IndexBuffer::GetIndexCount() const
    {
        return _indexCount;
    }
}
