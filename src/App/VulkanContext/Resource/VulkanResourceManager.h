#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "VulkanBuffer.h"

namespace Ailurus
{
	class VulkanDescriptorAllocator;

	class VulkanResourceManager : public NonCopyable, public NonMovable
	{
		using ResourcePtr = std::unique_ptr<VulkanResource, std::function<void(VulkanResource*)>>;
	public:
		~VulkanResourceManager();

	public:
		VulkanDeviceBuffer* CreateDeviceBuffer(vk::DeviceSize size, DeviceBufferUsage usage);
		VulkanHostBuffer* CreateHostBuffer(vk::DeviceSize size, HostBufferUsage usage, bool coherentWithGpu = true);
		void GarbageCollect();

	private:
		// Command buffer resources
		std::vector<ResourcePtr> _resources;
	};
} // namespace Ailurus