#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>
#include "VulkanBuffer.h"
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	

	class DataBufferUtil
	{
	public:
		DataBufferUtil() = delete;

	public:
		

		static void DestroyBuffer(CpuBuffer& cpuBuffer);
		static void DestroyBuffer(GpuBuffer& gpuBuffer);

		static void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
	};
} // namespace Ailurus