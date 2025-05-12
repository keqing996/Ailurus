#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>
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

	class VulkanDeviceBuffer : public VulkanResource
	{
		friend class VulkanResourceManager;

	public:
		VulkanDeviceBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem);

	public:
		const vk::DeviceSize realSize;
		const vk::Buffer buffer;
		const vk::DeviceMemory deviceMemory;
	};

	class VulkanHostBuffer : public VulkanResource
	{
		friend class VulkanResourceManager;

	public:
		VulkanHostBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem, void* addr);

	public:
		const vk::DeviceSize realSize;
		const vk::Buffer buffer;
		const vk::DeviceMemory deviceMemory;
		void* mappedAddr;
	};
} // namespace Ailurus