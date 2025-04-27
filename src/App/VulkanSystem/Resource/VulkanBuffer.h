#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>
#include "VulkanResource.h"

namespace Ailurus
{
	class VulkanDeviceBuffer : public VulkanResource
	{
	public:
		uint32_t GetHash() override;
		vk::Buffer GetBuffer() const;
		vk::DeviceMemory GetDeviceMemory() const;
		vk::DeviceSize GetSize() const;

	protected:
		vk::DeviceSize realSize;
		vk::Buffer buffer;
		vk::DeviceMemory deviceMemory;
	};

	class VulkanHostBuffer : public VulkanDeviceBuffer
	{
	public:
		uint8_t* GetMappedAddr() const;

	private:
		void* mappedAddr;
	};
} // namespace Ailurus