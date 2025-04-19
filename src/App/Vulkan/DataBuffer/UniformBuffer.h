#pragma once

#include "BufferType.h"
#include <Vulkan/vulkan.hpp>

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
		CpuBuffer _cpuBuffer;
		GpuBuffer _gpuBuffer;
	};

}