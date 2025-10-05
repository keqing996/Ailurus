#include "VulkanUniformBuffer.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/Resource/VulkanResourceManager.h"
#include "VulkanContext/Resource/DataBuffer/VulkanDeviceBuffer.h"
#include "VulkanContext/Resource/DataBuffer/VulkanHostBuffer.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"

namespace Ailurus
{
	VulkanUniformBuffer::VulkanUniformBuffer(size_t bufferSize)
		: _bufferSize(bufferSize)
	{
		for (auto i = 0; i < VulkanContext::GetParallelFrameCount(); i++)
			_backgroundBuffer.emplace_back(CreateBufferPair());
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		if (_currentBuffer.has_value())
		{
			_currentBuffer->cpuBuffer->MarkDelete();
			_currentBuffer->gpuBuffer->MarkDelete();
		}

		for (auto [cpuBuffer, gpuBuffer]: _backgroundBuffer)
		{
			cpuBuffer->MarkDelete();
			gpuBuffer->MarkDelete();
		}
	}

	void VulkanUniformBuffer::TransitionDataToGpu(VulkanCommandBuffer* pCommandBuffer)
	{
		EnsureCurrentBufferValid();

		pCommandBuffer->CopyBuffer(_currentBuffer->cpuBuffer, _currentBuffer->gpuBuffer, _bufferSize);
		pCommandBuffer->BufferMemoryBarrier(_currentBuffer->gpuBuffer, vk::AccessFlagBits::eTransferWrite,
			vk::AccessFlagBits::eUniformRead, vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eVertexShader);
	}

	VulkanDeviceBuffer* VulkanUniformBuffer::GetThisFrameDeviceBuffer()
	{
		EnsureCurrentBufferValid();
		return _currentBuffer->gpuBuffer;
	}

	VulkanUniformBuffer::BufferPair VulkanUniformBuffer::CreateBufferPair() const
	{
		auto pVkResMgr = VulkanContext::GetResourceManager();
		auto cpuBuffer = pVkResMgr->CreateHostBuffer(_bufferSize, HostBufferUsage::TransferSrc);
		auto gpuBuffer = pVkResMgr->CreateDeviceBuffer(_bufferSize, DeviceBufferUsage::Uniform);
		return { cpuBuffer, gpuBuffer };
	}

	void VulkanUniformBuffer::EnsureCurrentBufferValid()
	{
		if (_currentBuffer.has_value())
			return;

		auto& oldestBuffer = _backgroundBuffer.front();
		if (oldestBuffer.cpuBuffer->GetRefCount() == 0 && oldestBuffer.gpuBuffer->GetRefCount() == 0)
		{
			_currentBuffer = oldestBuffer;
			_backgroundBuffer.pop_front();
		}
		else
		{
			_currentBuffer = CreateBufferPair();
		}
	}

	void VulkanUniformBuffer::WriteData(uint32_t offset, const UniformValue& value)
	{
		EnsureCurrentBufferValid();

		auto pBeginPos = static_cast<uint8_t*>(_currentBuffer->cpuBuffer->mappedAddr) + offset;
		auto writeSize = value.GetSize();
		if (writeSize + offset > _bufferSize)
		{
			Logger::LogError("Write size {} at offset {} exceeds uniform buffer size {}", writeSize, offset, _bufferSize);
			return;
		}

		const void* pWriteData = nullptr;
		if (value.GetType() == UniformValueType::Mat4)
		{
			UniformValue newMat4(value.GetData().matrix4x4Value.Transpose());
			pWriteData = newMat4.GetDataPointer();
		}
		else
		{
			pWriteData = value.GetDataPointer();
		}

		std::memcpy(static_cast<void*>(pBeginPos), pWriteData, writeSize);
	}

	uint32_t VulkanUniformBuffer::GetBufferSize() const
	{
		return static_cast<uint32_t>(_bufferSize);
	}
} // namespace Ailurus