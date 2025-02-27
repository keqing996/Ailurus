#include "Ailurus/Graphics/DataBuffer/IndexBuffer.h"
#include "Ailurus/Graphics/Renderer.h"

namespace Ailurus
{
    IndexBuffer::IndexBuffer(const Renderer* pRenderer, std::vector<uint16_t> indexData)
        : Buffer(pRenderer)
        , _indexType(vk::IndexType::eUint16)
        , _buffer(nullptr)
        , _indexCount(indexData.size())
        , _bufferMemory(nullptr)
    {
        InternalCreate(reinterpret_cast<const char*>(indexData.data()),
            indexData.size() * sizeof(uint16_t));
    }

    IndexBuffer::IndexBuffer(const Renderer* pRenderer, std::vector<uint32_t> indexData)
        : Buffer(pRenderer)
        , _indexType(vk::IndexType::eUint32)
        , _buffer(nullptr)
        , _indexCount(indexData.size())
        , _bufferMemory(nullptr)
    {
        InternalCreate(reinterpret_cast<const char*>(indexData.data()),
                    indexData.size() * sizeof(uint32_t));
    }

    void IndexBuffer::InternalCreate(const char* dataBuffer, size_t dataSizeInBytes)
    {
        auto ret = CreateBuffer(BufferType::Index, dataBuffer, dataSizeInBytes);
        if (!ret)
            return;

        _buffer = ret->buffer;
        _bufferMemory = ret->deviceMemory;
    }

    IndexBuffer::~IndexBuffer()
    {
        const auto device = _pRenderer->GetLogicalDevice();
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
