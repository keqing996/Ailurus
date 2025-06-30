#pragma once

#include <array>
#include <Ailurus/Application/RenderSystem/Uniform/UniformValue.h>
#include "VulkanSystem/VulkanSystem.h"

namespace Ailurus
{
	class VulkanHostBuffer;
	class VulkanDeviceBuffer;

	class VulkanUniformBuffer
	{
	public:
		explicit VulkanUniformBuffer(size_t bufferSize);
		~VulkanUniformBuffer();

	public:
		uint8_t* GetWriteBeginPos() const;
		uint32_t GetBufferSize() const;
		void WriteData(uint32_t offset, const UniformValue& value) const;
		void TransitionDataToGpu() const;
		VulkanDeviceBuffer* GetThisFrameDeviceBuffer() const;

	private:
		size_t _bufferSize;
		std::array<VulkanHostBuffer*, VulkanSystem::PARALLEL_FRAME> _cpuBuffers;
		std::array<VulkanDeviceBuffer*, VulkanSystem::PARALLEL_FRAME> _gpuBuffers;
	};
} // namespace Ailurus