#pragma once

#include <array>
#include "VulkanSystem/VulkanSystem.h"
#include "BufferType.h"
#include "Ailurus/Application/Application.h"

namespace Ailurus
{
	class UniformBuffer
	{
	public:
		explicit UniformBuffer(size_t bufferSize);
		~UniformBuffer();

	public:
		uint8_t* GetWriteBeginPos() const;
		void TransitionDataToGpu() const;

	private:
		size_t _bufferSize;
		std::array<CpuBuffer, VulkanSystem::PARALLEL_FRAME> _cpuBuffers;
		std::array<GpuBuffer, VulkanSystem::PARALLEL_FRAME> _gpuBuffers;
	};

}