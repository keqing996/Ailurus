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
		VulkanDeviceBuffer* CreateDeviceBuffer(vk::DeviceSize size, GpuBufferUsage usage);
		VulkanHostBuffer* CreateCpuBuffer(vk::DeviceSize size, CpuBufferUsage usage, bool coherentWithGpu = true);

	private:
		friend class VulkanSystem;

	private:
		std::vector<ResourcePtr> _resources;
	};
} // namespace Ailurus