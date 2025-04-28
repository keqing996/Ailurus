#include <vulkan/vulkan_hash.hpp>
#include "VulkanBuffer.h"

namespace Ailurus
{

	VulkanDeviceBuffer::VulkanDeviceBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem)
		: realSize(size), buffer(buf), deviceMemory(mem)
	{
	}

	VulkanHostBuffer::VulkanHostBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem, void* addr)
		: realSize(size), buffer(buf), deviceMemory(mem), mappedAddr(addr)
	{
	}

	uint32_t VulkanDeviceBuffer::GetHash()
	{
		return std::hash<vk::Buffer>()(buffer);
	}

	uint32_t VulkanHostBuffer::GetHash()
	{
		return std::hash<vk::Buffer>()(buffer);
	}
} // namespace Ailurus