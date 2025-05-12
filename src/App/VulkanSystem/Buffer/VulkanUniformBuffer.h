#pragma once

#include <array>
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
		void TransitionDataToGpu() const;

	private:
		size_t _bufferSize;
		std::array<class VulkanHostBuffer*, VulkanSystem::PARALLEL_FRAME> _cpuBuffers;
		std::array<class VulkanDeviceBuffer*, VulkanSystem::PARALLEL_FRAME> _gpuBuffers;
	};

}