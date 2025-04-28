#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>
#include "VulkanResource.h"
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(CpuBufferUsage,
		TransferSrc)

	REFLECTION_ENUM(GpuBufferUsage,
		Vertex,
		Index,
		Uniform)

	class VulkanDeviceBuffer : public VulkanResource
	{
		friend class VulkanResourceManager;

	public:
		VulkanDeviceBuffer(vk::DeviceSize size, vk::Buffer buf, vk::DeviceMemory mem);
		uint32_t GetHash() override;

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
		uint32_t GetHash() override;

	public:
		const vk::DeviceSize realSize;
		const vk::Buffer buffer;
		const vk::DeviceMemory deviceMemory;
		const void* mappedAddr;
	};
} // namespace Ailurus