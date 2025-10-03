#pragma once

#include "VulkanDataBuffer.h"

namespace Ailurus
{
    class VulkanHostBuffer : public VulkanDataBuffer
	{
	public:
		VulkanHostBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem, void* addr)
            : VulkanDataBuffer(size, buf, mem), mappedAddr(addr)
	    {
	    }

	public:
		void* mappedAddr;

    private:
        friend class VulkanResourceManager;
        static VulkanResourcePtr Create(vk::DeviceSize size, HostBufferUsage usage, bool coherentWithGpu);
	};
}