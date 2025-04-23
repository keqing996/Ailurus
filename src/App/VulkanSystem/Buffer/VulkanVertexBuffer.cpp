#include "VulkanVertexBuffer.h"
#include "DataBufferUtil.h"

namespace Ailurus
{
    VulkanVertexBuffer::VulkanVertexBuffer(const void* vertexData, size_t sizeInBytes)
        : _sizeInBytes(sizeInBytes)
    {
    	// Create gpu buffer
    	const auto retCreateGpuBuffer = DataBufferUtil::CreateGpuBuffer(sizeInBytes, GpuBufferUsage::Vertex);
    	if (!retCreateGpuBuffer.has_value())
    		return;

    	_buffer = *retCreateGpuBuffer;

    	// Create cpu stage buffer
    	const auto retCreateStageBuffer = DataBufferUtil::CreateCpuBuffer(sizeInBytes, CpuBufferUsage::TransferSrc);
    	if (!retCreateStageBuffer.has_value())
    		return;

    	CpuBuffer stageBuffer = *retCreateStageBuffer;

    	// Cpu -> Cpu buffer
    	std::memcpy(stageBuffer.mappedAddr, vertexData, sizeInBytes);

    	// Cpu buffer -> Gpu buffer
    	DataBufferUtil::CopyBuffer(stageBuffer.buffer, _buffer.buffer, sizeInBytes);

    	// Destroy stage cpu buffer
    	DataBufferUtil::DestroyBuffer(stageBuffer);
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
    	DataBufferUtil::DestroyBuffer(_buffer);
    }

    vk::Buffer VulkanVertexBuffer::GetBuffer() const
    {
        return _buffer.buffer;
    }

    size_t VulkanVertexBuffer::GetSize() const
    {
        return _sizeInBytes;
    }
}
