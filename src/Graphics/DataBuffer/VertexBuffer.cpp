#include "Ailurus/Graphics/DataBuffer/VertexBuffer.h"

namespace Ailurus
{
    VertexBuffer::VertexBuffer(const Renderer* pRenderer, const std::vector<char>& vertexData)
        : Buffer(pRenderer)
        , _buffer(nullptr)
        , _bufferMemory(nullptr)
    {
        auto ret = CreateBuffer(BufferType::Vertex, vertexData);
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
}
