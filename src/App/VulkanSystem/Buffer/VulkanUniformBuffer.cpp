#include "VulkanUniformBuffer.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"
#include "VulkanSystem/Resource/VulkanResourceManager.h"
#include "VulkanSystem/CommandBuffer/VulkanCommandBuffer.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	VulkanUniformBuffer::VulkanUniformBuffer(size_t bufferSize)
		: _bufferSize(bufferSize)
	{
		auto pVulkanResManager = Application::Get<VulkanSystem>()->GetResourceManager();

		for (auto i = 0; i < Application::Get<VulkanSystem>()->PARALLEL_FRAME; i++)
		{
			auto cpuBuffer = pVulkanResManager->CreateHostBuffer(bufferSize, HostBufferUsage::TransferSrc);
			if (cpuBuffer == nullptr)
				return;

			_cpuBuffers[i] = cpuBuffer;

			auto gpuBuffer = pVulkanResManager->CreateDeviceBuffer(bufferSize, DeviceBufferUsage::Uniform);
			if (gpuBuffer == nullptr)
				return;

			_gpuBuffers[i] = gpuBuffer;
		}
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		for (auto gpuBuffer : _gpuBuffers)
			gpuBuffer->MarkDelete();

		for (auto cpuBuffer : _cpuBuffers)
			cpuBuffer->MarkDelete();
	}

	uint8_t* VulkanUniformBuffer::GetWriteBeginPos() const
	{
		auto index = Application::Get<VulkanSystem>()->GetCurrentParallelFrameIndex();
		return static_cast<uint8_t*>(_cpuBuffers[index]->mappedAddr);
	}

	void VulkanUniformBuffer::TransitionDataToGpu() const
	{
		auto index = Application::Get<VulkanSystem>()->GetCurrentParallelFrameIndex();

		VulkanCommandBuffer* pCommandBuffer = Application::Get<VulkanSystem>()->GetFrameContext()->GetRecordingCommandBuffer();
		pCommandBuffer->CopyBuffer(_cpuBuffers[index], _gpuBuffers[index], _bufferSize);
		pCommandBuffer->BufferMemoryBarrier(_gpuBuffers[index], vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eUniformRead, 
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eVertexShader);
	}

	VulkanDeviceBuffer* VulkanUniformBuffer::GetThisFrameDeviceBuffer() const
	{
		auto index = Application::Get<VulkanSystem>()->GetCurrentParallelFrameIndex();
		return _gpuBuffers[index];
	}

	void VulkanUniformBuffer::WriteData(uint32_t offset, const UniformValue& value) const
	{
		if (offset >= _bufferSize)
		{
			Logger::LogError("Offset {} is out of range for uniform buffer size {}", offset, _bufferSize);
			return;
		}

		auto pBeginPos = GetWriteBeginPos() + offset;
		auto visitor = [pBeginPos](auto&& arg) 
		{
			using T = std::decay_t<decltype(arg)>;
			std::memcpy(static_cast<void*>(pBeginPos), &arg, sizeof(T));
		};

		std::visit(visitor, value);
	}

	uint32_t VulkanUniformBuffer::GetBufferSize() const
	{
		return static_cast<uint32_t>(_bufferSize);
	}
} // namespace Ailurus