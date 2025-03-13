#include "VertexBuffer.h"
#include "DataBufferUtil.h"
#include "Vulkan/Context/VulkanContext.h"

namespace Ailurus
{
    VertexBuffer::VertexBuffer(const char* vertexData, size_t dataSizeInBytes)
        : _buffer(nullptr)
        , _size(dataSizeInBytes)
        , _bufferMemory(nullptr)
    {
        auto ret = DataBufferUtil::CreateBuffer(BufferType::Vertex, vertexData, dataSizeInBytes);
        if (!ret)
            return;

        _buffer = ret->buffer;
        _bufferMemory = ret->deviceMemory;
    }

    VertexBuffer::~VertexBuffer()
    {
        const auto device = VulkanContext::GetDevice();
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
