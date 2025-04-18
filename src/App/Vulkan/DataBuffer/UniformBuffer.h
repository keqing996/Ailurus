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
		vk::Buffer GetBuffer() const;
		void UpdateData(size_t offset, const void* data, size_t updateDataSize) const;

	private:
		CpuBuffer _buffer;
	};

}