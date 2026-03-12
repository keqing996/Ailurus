#pragma once

#include "VulkanContext/VulkanPch.h"
#include "HostBufferUsage.h"
#include "DeviceBufferUsage.h"
#include "VulkanContext/Resource/VulkanResource.h"

namespace Ailurus
{
    class VulkanDataBuffer : public VulkanResource
	{
    public:
        struct BufferMemoryRequirement
	    {
	    	vk::MemoryRequirements requirements;
	    	uint32_t memTypeIndex;
	    };
    
	    struct CreatedBuffer
	    {
	    	vk::DeviceSize realSize;
	    	vk::Buffer buffer;
	    	vk::DeviceMemory deviceMem;
	    };

	public:
		VulkanDataBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem)
            : realSize(size), buffer(buf), deviceMemory(mem)
	    {
	    }

	public:
		const vk::DeviceSize realSize;
		const vk::Buffer buffer;
		const vk::DeviceMemory deviceMemory;

    protected:
        static auto GetBufferMemoryRequirement(vk::Buffer buffer, vk::MemoryPropertyFlags propertyFlag) -> std::optional<BufferMemoryRequirement>;
        static auto CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlag, vk::MemoryPropertyFlags propertyFlag) -> std::optional<CreatedBuffer>;
	};
}