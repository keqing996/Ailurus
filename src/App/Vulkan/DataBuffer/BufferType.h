#pragma once

#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
    REFLECTION_ENUM(BufferType,
        Vertex,
        Index,
        Uniform);

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