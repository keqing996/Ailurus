#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	class DataBufferUtil
	{
	public:
		DataBufferUtil() = delete;

	public:
		static void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
	};
} // namespace Ailurus