#include <vulkan/vulkan_hash.hpp>
#include "VulkanBuffer.h"

namespace Ailurus
{
	VulkanDataBuffer::VulkanDataBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem)
		: realSize(size), buffer(buf), deviceMemory(mem)
	{

	}

	VulkanDeviceBuffer::VulkanDeviceBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem)
		: VulkanDataBuffer(size, buf, mem)
	{
	}

	VulkanHostBuffer::VulkanHostBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem, void* addr)
		: VulkanDataBuffer(size, buf, mem), mappedAddr(addr)
	{
	}
} // namespace Ailurus