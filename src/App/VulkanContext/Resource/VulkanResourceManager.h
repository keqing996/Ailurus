#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "VulkanResource.h"
#include "DataBuffer/HostBufferUsage.h"
#include "DataBuffer/DeviceBufferUsage.h"

namespace Ailurus
{
	class VulkanDescriptorAllocator;
	class VulkanImage;
	class VulkanSampler;
	class Image;
	class VulkanDeviceBuffer;
	class VulkanHostBuffer;

	class VulkanResourceManager : public NonCopyable, public NonMovable
	{
	public:
		~VulkanResourceManager();

	public:
		auto CreateDeviceBuffer(vk::DeviceSize size, 
			DeviceBufferUsage usage, 
			const std::string& debugName = "Unnamed resource") -> VulkanDeviceBuffer*;

		auto CreateHostBuffer(vk::DeviceSize size, 
			HostBufferUsage usage, 
			bool coherentWithGpu = true, 
			const std::string& debugName = "Unnamed resource") -> VulkanHostBuffer*;

		auto CreateImage(const Image& image, 
			const std::string& debugName = "Unnamed resource") -> VulkanImage*;

		auto CreateSampler(const std::string& debugName = "Unnamed resource") -> VulkanSampler*;
		
		auto GarbageCollect() -> void;

	private:
		// Command buffer resources
		std::vector<VulkanResourcePtr> _resources;
	};
} // namespace Ailurus