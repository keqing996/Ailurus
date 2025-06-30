#pragma once

#include <array>
#include <Ailurus/Application/RenderSystem/Uniform/UniformValue.h>
#include "VulkanSystem/VulkanSystem.h"

namespace Ailurus
{
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

	private:
		size_t _bufferSize;
		std::array<class VulkanHostBuffer*, VulkanSystem::PARALLEL_FRAME> _cpuBuffers;
		std::array<class VulkanDeviceBuffer*, VulkanSystem::PARALLEL_FRAME> _gpuBuffers;
	};
} // namespace Ailurus