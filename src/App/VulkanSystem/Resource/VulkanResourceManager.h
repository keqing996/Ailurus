#pragma once

#include <vector>
#include <memory>
#include <functional>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "VulkanBuffer.h"

namespace Ailurus
{
	class VulkanResourceManager : public NonCopyable, public NonMovable
	{
		using ResourcePtr = std::unique_ptr<VulkanResource, std::function<void(VulkanResource*)>>;
	public:
		VulkanDeviceBuffer* CreateDeviceBuffer(vk::DeviceSize size, DeviceBufferUsage usage);
		VulkanHostBuffer* CreateHostBuffer(vk::DeviceSize size, HostBufferUsage usage, bool coherentWithGpu = true);

		void GarbageCollect();

	private:
		friend class VulkanSystem;

	private:
		std::vector<ResourcePtr> _resources;
	};
} // namespace Ailurus