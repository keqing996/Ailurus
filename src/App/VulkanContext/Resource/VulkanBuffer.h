#pragma once

#include "VulkanContext/VulkanPch.h"
#include <cstdint>
#include "VulkanResource.h"
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(HostBufferUsage,
		TransferSrc)

	REFLECTION_ENUM(DeviceBufferUsage,
		Vertex,
		Index,
		Uniform)

	class VulkanDataBuffer : public VulkanResource
	{
	public:
		VulkanDataBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem);

	public:
		const vk::DeviceSize realSize;
		const vk::Buffer buffer;
		const vk::DeviceMemory deviceMemory;
	};

	class VulkanDeviceBuffer : public VulkanDataBuffer
	{
		friend class VulkanResourceManager;

	public:
		VulkanDeviceBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem);
	};

	class VulkanHostBuffer : public VulkanDataBuffer
	{
		friend class VulkanResourceManager;

	public:
		VulkanHostBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem, void* addr);

	public:
		void* mappedAddr;
	};
} // namespace Ailurus