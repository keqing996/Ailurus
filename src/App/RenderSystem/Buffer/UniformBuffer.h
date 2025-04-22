#pragma once

#include <array>
#include "Render/Context/RhiContext.h"
#include "BufferType.h"

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
		std::array<CpuBuffer, RhiContext::PARALLEL_FRAME> _cpuBuffers;
		std::array<GpuBuffer, RhiContext::PARALLEL_FRAME> _gpuBuffers;
	};

}