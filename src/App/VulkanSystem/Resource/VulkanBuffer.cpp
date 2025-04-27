#include "VulkanBuffer.h"
#include <vulkan/vulkan_hash.hpp>

namespace Ailurus
{

	vk::Buffer VulkanDeviceBuffer::GetBuffer() const
	{
		return buffer;
	}

	vk::DeviceMemory VulkanDeviceBuffer::GetDeviceMemory() const
	{
		return deviceMemory;
	}

	vk::DeviceSize VulkanDeviceBuffer::GetSize() const
	{
		return realSize;
	}

	uint8_t* VulkanHostBuffer::GetMappedAddr() const
	{
		return static_cast<uint8_t*>(mappedAddr);
	}

	uint32_t VulkanDeviceBuffer::GetHash()
	{
		return std::hash<vk::Buffer>()(buffer);
	}
} // namespace Ailurus