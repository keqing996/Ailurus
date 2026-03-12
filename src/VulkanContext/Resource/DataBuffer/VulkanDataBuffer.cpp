#include "VulkanDataBuffer.h"
#include "VulkanContext/VulkanContext.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    std::optional<VulkanDataBuffer::BufferMemoryRequirement>
	VulkanDataBuffer::GetBufferMemoryRequirement(vk::Buffer buffer, vk::MemoryPropertyFlags propertyFlag)
	{
		vk::MemoryRequirements memRequirements = VulkanContext::GetDevice().getBufferMemoryRequirements(buffer);

		// Find a memory type
		std::optional<uint32_t> memoryTypeIndex = std::nullopt;
		vk::PhysicalDeviceMemoryProperties memProperties = VulkanContext::GetPhysicalDevice().getMemoryProperties();
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((memRequirements.memoryTypeBits & (1 << i))
				&& (memProperties.memoryTypes[i].propertyFlags & propertyFlag) == propertyFlag)
			{
				memoryTypeIndex = i;
				break;
			}
		}

		if (memoryTypeIndex.has_value())
			return BufferMemoryRequirement{ memRequirements, *memoryTypeIndex };

		return std::nullopt;
	}

    std::optional<VulkanDataBuffer::CreatedBuffer>
	VulkanDataBuffer::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlag, vk::MemoryPropertyFlags propertyFlag)
	{
		const auto device = VulkanContext::GetDevice();

		// Create buffer
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.setSize(size)
			.setUsage(usageFlag)
			.setSharingMode(vk::SharingMode::eExclusive);

		try
		{
			const vk::Buffer buffer = device.createBuffer(bufferInfo);

			// Get gpu memory requirement
			const std::optional<BufferMemoryRequirement> memoryRequirement = GetBufferMemoryRequirement(buffer, propertyFlag);
			if (!memoryRequirement.has_value())
			{
				device.destroyBuffer(buffer);
				Logger::LogError("Failed to get buffer memory requirement when creating buffer.");
				return std::nullopt;
			}

			// Alloc memory
			vk::MemoryAllocateInfo allocInfo;
			allocInfo.setAllocationSize(memoryRequirement->requirements.size)
				.setMemoryTypeIndex(memoryRequirement->memTypeIndex);

			const vk::DeviceMemory deviceMem = device.allocateMemory(allocInfo);
			device.bindBufferMemory(buffer, deviceMem, 0);

			return CreatedBuffer{ memoryRequirement->requirements.size, buffer, deviceMem };
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create buffer: {}", e.what());
			return std::nullopt;
		}
	}
}