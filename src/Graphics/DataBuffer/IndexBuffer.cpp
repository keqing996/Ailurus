#include "Ailurus/Graphics/DataBuffer/IndexBuffer.h"

namespace Ailurus
{
    IndexBuffer::IndexBuffer(const Renderer* pRenderer, const std::vector<char>& indexData)
        : Buffer(pRenderer)
        , _buffer(nullptr)
        , _bufferMemory(nullptr)
    {
        auto ret = CreateBuffer(BufferType::Index, indexData);
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

    vk::Buffer IndexBuffer::GetBuffer() const
    {
        return _buffer;
    }
}
