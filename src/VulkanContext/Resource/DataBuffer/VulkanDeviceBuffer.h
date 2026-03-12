#pragma once

#include "VulkanDataBuffer.h"

namespace Ailurus
{
    class VulkanDeviceBuffer : public VulkanDataBuffer
	{
	public:
		VulkanDeviceBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem)
            : VulkanDataBuffer(size, buf, mem)
	    {
	    }

    private:
        friend class VulkanResourceManager;
        static VulkanResourcePtr Create(vk::DeviceSize size, DeviceBufferUsage usage);
	};
}