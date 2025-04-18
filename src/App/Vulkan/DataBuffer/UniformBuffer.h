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

		vk::Buffer GetBuffer() const;

	private:
		CpuBuffer _buffer;
	};

}