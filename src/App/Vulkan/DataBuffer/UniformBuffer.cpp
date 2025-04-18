#include "UniformBuffer.h"
#include "DataBufferUtil.h"

namespace Ailurus
{
	UniformBuffer::UniformBuffer(size_t bufferSize)
	{
		const auto ret = DataBufferUtil::CreateCpuBuffer(bufferSize, CpuBufferUsage::Uniform);
		if (ret.has_value())
			_buffer = *ret;
	}

	UniformBuffer::~UniformBuffer()
	{
		DataBufferUtil::DestroyBuffer(_buffer);
	}

	vk::Buffer UniformBuffer::GetBuffer() const
	{
		return _buffer.buffer;
	}
} // namespace Ailurus