#include "UniformBuffer.h"
#include "DataBufferUtil.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	UniformBuffer::UniformBuffer(size_t bufferSize)
		: _bufferSize(bufferSize)
	{
		for (auto i = 0; i < RhiContext::PARALLEL_FRAME; i++)
		{
			const auto retCpuBuffer = DataBufferUtil::CreateCpuBuffer(bufferSize, CpuBufferUsage::TransferSrc, false);
			if (!retCpuBuffer.has_value())
				return;

			_cpuBuffers[i] = *retCpuBuffer;

			const auto retGpuBuffer = DataBufferUtil::CreateGpuBuffer(bufferSize, GpuBufferUsage::Uniform);
			if (!retGpuBuffer.has_value())
				return;

			_gpuBuffers[i] = *retGpuBuffer;
		}
	}

	UniformBuffer::~UniformBuffer()
	{
		for (auto gpuBuffer : _gpuBuffers)
			DataBufferUtil::DestroyBuffer(gpuBuffer);

		for (auto cpuBuffer : _cpuBuffers)
			DataBufferUtil::DestroyBuffer(cpuBuffer);
	}

	uint8_t* UniformBuffer::GetWriteBeginPos() const
	{
		return static_cast<uint8_t*>(_cpuBuffers[RhiContext::GetCurrentFrameIndex()].mappedAddr);
	}

	void UniformBuffer::TransitionDataToGpu() const
	{
		DataBufferUtil::CopyBuffer(
			_cpuBuffers[RhiContext::GetCurrentFrameIndex()].buffer,
			_gpuBuffers[RhiContext::GetCurrentFrameIndex()].buffer,
			_bufferSize);
	}
} // namespace Ailurus