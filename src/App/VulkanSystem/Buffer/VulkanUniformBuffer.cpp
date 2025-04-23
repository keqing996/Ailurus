#include "VulkanUniformBuffer.h"
#include "DataBufferUtil.h"

namespace Ailurus
{
	VulkanUniformBuffer::VulkanUniformBuffer(size_t bufferSize)
		: _bufferSize(bufferSize)
	{
		for (auto i = 0; i < Application::Get<VulkanSystem>()->PARALLEL_FRAME; i++)
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

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		for (auto gpuBuffer : _gpuBuffers)
			DataBufferUtil::DestroyBuffer(gpuBuffer);

		for (auto cpuBuffer : _cpuBuffers)
			DataBufferUtil::DestroyBuffer(cpuBuffer);
	}

	uint8_t* VulkanUniformBuffer::GetWriteBeginPos() const
	{
		return static_cast<uint8_t*>(_cpuBuffers[Application::Get<VulkanSystem>()->GetCurrentFrameIndex()].mappedAddr);
	}

	void VulkanUniformBuffer::TransitionDataToGpu() const
	{
		DataBufferUtil::CopyBuffer(
			_cpuBuffers[Application::Get<VulkanSystem>()->GetCurrentFrameIndex()].buffer,
			_gpuBuffers[Application::Get<VulkanSystem>()->GetCurrentFrameIndex()].buffer,
			_bufferSize);
	}
} // namespace Ailurus