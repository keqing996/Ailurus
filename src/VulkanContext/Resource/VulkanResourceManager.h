#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "VulkanResource.h"
#include "DataBuffer/HostBufferUsage.h"
#include "DataBuffer/DeviceBufferUsage.h"
#include "Image/VulkanImage.h"
#include "Image/VulkanSampler.h"

namespace Ailurus
{
	class VulkanDescriptorAllocator;
	class VulkanSampler;
	class Image;
	class VulkanDeviceBuffer;
	class VulkanHostBuffer;

	class VulkanResourceManager : public NonCopyable, public NonMovable
	{
	public:
		~VulkanResourceManager();

	public:
		VulkanDeviceBuffer* CreateDeviceBuffer(vk::DeviceSize size, DeviceBufferUsage usage);
		VulkanHostBuffer* CreateHostBuffer(vk::DeviceSize size, HostBufferUsage usage, bool coherentWithGpu = true);
		VulkanImage* CreateImage(const Image& image);
		VulkanImage* CreateImageFromConfig(const VulkanImageCreateConfig& config,
			const void* pixelData, size_t dataSize);
		VulkanSampler* CreateSampler();
		VulkanSampler* CreateSampler(const VulkanSamplerCreateConfig& config);
		void GarbageCollect();

	private:
		// Command buffer resources
		std::vector<VulkanResourcePtr> _resources;
	};
} // namespace Ailurus