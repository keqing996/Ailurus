#include "UniformBuffer.h"
#include "DataBufferUtil.h"
#include "Ailurus/Utility/Logger.h"

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

	void UniformBuffer::UpdateData(size_t offset, const void* data, size_t updateDataSize) const
	{
		if (_buffer.mappedAddr == nullptr)
		{
			Logger::LogError("Try write an unmapped uniform buffer");
			return;
		}

		std::memcpy(_buffer.mappedAddr + offset, data, updateDataSize);
	}
} // namespace Ailurus