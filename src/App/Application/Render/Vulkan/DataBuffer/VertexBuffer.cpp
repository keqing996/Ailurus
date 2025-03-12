#include "Ailurus/Graphics/DataBuffer/VertexBuffer.h"
#include "Ailurus/Graphics/Renderer.h"

namespace Ailurus
{
    VertexBuffer::VertexBuffer(const Renderer* pRenderer, const char* vertexData, size_t dataSizeInBytes)
        : Buffer(pRenderer)
        , _buffer(nullptr)
        , _size(dataSizeInBytes)
        , _bufferMemory(nullptr)
    {
        auto ret = CreateBuffer(BufferType::Vertex, vertexData, dataSizeInBytes);
        if (!ret)
            return;

        _buffer = ret->buffer;
        _bufferMemory = ret->deviceMemory;
    }

    VertexBuffer::~VertexBuffer()
    {
        const auto device = _pRenderer->GetLogicalDevice();
        device.freeMemory(_bufferMemory);
        device.destroyBuffer(_buffer);
    }

    vk::Buffer VertexBuffer::GetBuffer() const
    {
        return _buffer;
    }

    size_t VertexBuffer::GetSize() const
    {
        return _size;
    }
}
