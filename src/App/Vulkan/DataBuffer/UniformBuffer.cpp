#include "UniformBuffer.h"
#include "DataBufferUtil.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	UniformBuffer::UniformBuffer(size_t bufferSize)
		: _bufferSize(bufferSize)
	{
		const auto retCpuBuffer = DataBufferUtil::CreateCpuBuffer(bufferSize, CpuBufferUsage::TransferSrc, false);
		if (!retCpuBuffer.has_value())
			return;

		_cpuBuffer = *retCpuBuffer;

		const auto retGpuBuffer = DataBufferUtil::CreateGpuBuffer(bufferSize, GpuBufferUsage::Uniform);
		if (!retGpuBuffer.has_value())
			return;

		_gpuBuffer = *retGpuBuffer;
	}

	UniformBuffer::~UniformBuffer()
	{
		DataBufferUtil::DestroyBuffer(_gpuBuffer);
		DataBufferUtil::DestroyBuffer(_cpuBuffer);
	}

	uint8_t* UniformBuffer::GetWriteBeginPos() const
	{
		return static_cast<uint8_t*>(_cpuBuffer.mappedAddr);
	}

	void UniformBuffer::TransitionDataToGpu() const
	{
		DataBufferUtil::CopyBuffer(_cpuBuffer.buffer, _gpuBuffer.buffer, _bufferSize);
	}
} // namespace Ailurus