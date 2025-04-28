#pragma once

#include <vector>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "VulkanBuffer.h"

namespace Ailurus
{
	class VulkanResourceManager : public NonCopyable, public NonMovable
	{
	public:
		VulkanDeviceBuffer* CreateDeviceBuffer(vk::DeviceSize size, GpuBufferUsage usage);
		VulkanHostBuffer* CreateCpuBuffer(vk::DeviceSize size, CpuBufferUsage usage, bool coherentWithGpu = true);

	private:
		friend class VulkanSystem;

	private:
		std::vector<std::unique_ptr<VulkanResource>> _resources;
	};
} // namespace Ailurus