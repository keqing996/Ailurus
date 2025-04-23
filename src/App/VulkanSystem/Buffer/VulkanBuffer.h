#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	struct GpuBuffer
	{
		vk::DeviceSize realSize;
		vk::Buffer buffer;
		vk::DeviceMemory deviceMemory;
	};

	struct CpuBuffer
	{
		vk::DeviceSize realSize;
		vk::Buffer buffer;
		vk::DeviceMemory deviceMemory;
		void* mappedAddr;
	};
}