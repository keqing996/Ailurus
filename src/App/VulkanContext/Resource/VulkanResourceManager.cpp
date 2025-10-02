#include "VulkanResourceManager.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanContext/Resource/VulkanBuffer.h"
#include "VulkanContext/Resource/VulkanResource.h"
#include "VulkanContext/Resource/VulkanImage.h"
#include "VulkanContext/Resource/VulkanSampler.h"
#include "VulkanContext/VulkanContext.h"
#include "Ailurus/Utility/Image.h"

namespace Ailurus
{
	struct BufferMemoryRequirement
	{
		vk::MemoryRequirements requirements;
		uint32_t memTypeIndex;
	};
	
	struct CreatedBuffer
	{
		vk::DeviceSize realSize;
		vk::Buffer buffer;
		vk::DeviceMemory deviceMem;
	};

	static std::optional<BufferMemoryRequirement>
	GetBufferMemoryRequirement(vk::Buffer buffer, vk::MemoryPropertyFlags propertyFlag)
	{
		vk::MemoryRequirements memRequirements = VulkanContext::GetDevice().getBufferMemoryRequirements(buffer);

		// Find a memory type
		std::optional<uint32_t> memoryTypeIndex = std::nullopt;
		vk::PhysicalDeviceMemoryProperties memProperties = VulkanContext::GetPhysicalDevice().getMemoryProperties();
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((memRequirements.memoryTypeBits & (1 << i))
				&& (memProperties.memoryTypes[i].propertyFlags & propertyFlag) == propertyFlag)
			{
				memoryTypeIndex = i;
				break;
			}
		}

		if (memoryTypeIndex.has_value())
			return BufferMemoryRequirement{ memRequirements, *memoryTypeIndex };

		return std::nullopt;
	}

	static std::optional<CreatedBuffer>
	CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlag, vk::MemoryPropertyFlags propertyFlag)
	{
		const auto device = VulkanContext::GetDevice();

		// Create buffer
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.setSize(size)
			.setUsage(usageFlag)
			.setSharingMode(vk::SharingMode::eExclusive);

		try
		{
			const vk::Buffer buffer = device.createBuffer(bufferInfo);

			// Get gpu memory requirement
			const std::optional<BufferMemoryRequirement> memoryRequirement = GetBufferMemoryRequirement(buffer, propertyFlag);
			if (!memoryRequirement.has_value())
			{
				device.destroyBuffer(buffer);
				Logger::LogError("Failed to get buffer memory requirement when creating buffer.");
				return std::nullopt;
			}

			// Alloc memory
			vk::MemoryAllocateInfo allocInfo;
			allocInfo.setAllocationSize(memoryRequirement->requirements.size)
				.setMemoryTypeIndex(memoryRequirement->memTypeIndex);

			const vk::DeviceMemory deviceMem = device.allocateMemory(allocInfo);
			device.bindBufferMemory(buffer, deviceMem, 0);

			return CreatedBuffer{ memoryRequirement->requirements.size, buffer, deviceMem };
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create buffer: {}", e.what());
			return std::nullopt;
		}
	}

	template <typename T>
	void DeleteVulkanResource(VulkanResource* pResource);

	template <>
	void DeleteVulkanResource<VulkanHostBuffer>(VulkanResource* pResource)
	{
		auto ptr = static_cast<VulkanHostBuffer*>(pResource);
		const auto device = VulkanContext::GetDevice();

		try
		{
			if (ptr->mappedAddr != nullptr)
				device.unmapMemory(ptr->deviceMemory);

			// Destroy buffer first, then device memory.
			if (ptr->buffer != nullptr)
				device.destroyBuffer(ptr->buffer);

			if (ptr->deviceMemory != nullptr)
				device.freeMemory(ptr->deviceMemory);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to delete host buffer resource: {}", e.what());
		}

		delete ptr;
	}

	template <>
	void DeleteVulkanResource<VulkanDeviceBuffer>(VulkanResource* pResource)
	{
		auto ptr = static_cast<VulkanDeviceBuffer*>(pResource);
		const auto device = VulkanContext::GetDevice();

		try
		{
			// Destroy buffer first, then device memory.
			if (ptr->buffer != nullptr)
				device.destroyBuffer(ptr->buffer);

			if (ptr->deviceMemory != nullptr)
				device.freeMemory(ptr->deviceMemory);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to delete device buffer resource: {}", e.what());
		}

		delete ptr;
	}

	template <>
	void DeleteVulkanResource<VulkanImage>(VulkanResource* pResource)
	{
		delete static_cast<VulkanImage*>(pResource);
	}

	template <>
	void DeleteVulkanResource<VulkanSampler>(VulkanResource* pResource)
	{
		delete static_cast<VulkanSampler*>(pResource);
	}

	VulkanResourceManager::~VulkanResourceManager()
	{
	}

	VulkanDeviceBuffer* VulkanResourceManager::CreateDeviceBuffer(vk::DeviceSize size, DeviceBufferUsage usage)
	{
		vk::BufferUsageFlags usageFlag;
		usageFlag |= vk::BufferUsageFlagBits::eTransferDst;
		switch (usage)
		{
			case DeviceBufferUsage::Vertex:
				usageFlag |= vk::BufferUsageFlagBits::eVertexBuffer;
				break;
			case DeviceBufferUsage::Index:
				usageFlag |= vk::BufferUsageFlagBits::eIndexBuffer;
				break;
			case DeviceBufferUsage::Uniform:
				usageFlag |= vk::BufferUsageFlagBits::eUniformBuffer;
				break;
			default:
				Logger::LogError("Unknown gpu buffer usage type: {}", EnumReflection<DeviceBufferUsage>::ToString(usage));
				return nullptr;
		}

		constexpr vk::MemoryPropertyFlags propertyFlag = vk::MemoryPropertyFlagBits::eDeviceLocal;
		const std::optional<CreatedBuffer> bufferRet = CreateBuffer(size, usageFlag, propertyFlag);
		if (!bufferRet.has_value())
			return nullptr;

		VulkanDeviceBuffer* pBufferRaw = new VulkanDeviceBuffer(bufferRet->realSize, bufferRet->buffer, bufferRet->deviceMem);
		_resources.push_back(ResourcePtr(pBufferRaw, &DeleteVulkanResource<VulkanDeviceBuffer>));

		return pBufferRaw;
	}

	VulkanHostBuffer* VulkanResourceManager::CreateHostBuffer(vk::DeviceSize size, HostBufferUsage usage, bool coherentWithGpu)
	{
		vk::BufferUsageFlags usageFlag;
		switch (usage)
		{
			case HostBufferUsage::TransferSrc:
				usageFlag |= vk::BufferUsageFlagBits::eTransferSrc;
				break;
			default:
				Logger::LogError("Unknown cpu buffer usage type: {}", EnumReflection<HostBufferUsage>::ToString(usage));
				return nullptr;
		}

		vk::MemoryPropertyFlags propertyFlag = vk::MemoryPropertyFlagBits::eHostVisible;
		if (coherentWithGpu)
			propertyFlag |= vk::MemoryPropertyFlagBits::eHostCoherent;

		const std::optional<CreatedBuffer> bufferRet = CreateBuffer(size, usageFlag, propertyFlag);
		if (!bufferRet.has_value())
			return nullptr;

		// Map memory
		try
		{
			void* mappedAddr = VulkanContext::GetDevice().mapMemory(bufferRet->deviceMem, 0, size, {});
			VulkanHostBuffer* pBufferRaw = new VulkanHostBuffer(bufferRet->realSize, bufferRet->buffer, bufferRet->deviceMem, mappedAddr);
			_resources.push_back(ResourcePtr(pBufferRaw, &DeleteVulkanResource<VulkanHostBuffer>));
			return pBufferRaw;
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to map memory: {}", e.what());
			return nullptr;
		}
	}

	void VulkanResourceManager::GarbageCollect()
	{
		// Mark
		static std::vector<uint64_t> needDeletedResourceIndex;
		needDeletedResourceIndex.clear();
		for (auto i = 0; i < _resources.size(); i++)
		{
			if (_resources[i]->IsMarkDeleted() && _resources[i]->GetRefCount() == 0)
				needDeletedResourceIndex.push_back(i);
		}

		// Release
		for (auto index : needDeletedResourceIndex)
			_resources[index] = nullptr;

		// Clean up
		static std::vector<ResourcePtr> resourcesBuffer;
		resourcesBuffer.clear();
		for (auto i = 0; i < _resources.size(); i++)
		{
			if (_resources[i] != nullptr)
				resourcesBuffer.push_back(std::move(_resources[i]));
		}

		std::swap(_resources, resourcesBuffer);
	}

	VulkanImage* VulkanResourceManager::CreateImage(const Image& image)
	{
		const auto device = VulkanContext::GetDevice();
		const auto [width, height] = image.GetPixelSize();
		const vk::Format format = vk::Format::eR8G8B8A8Srgb;

		try
		{
			// Create image
			vk::ImageCreateInfo imageInfo;
			imageInfo.setImageType(vk::ImageType::e2D)
				.setFormat(format)
				.setExtent(vk::Extent3D(width, height, 1))
				.setMipLevels(1)
				.setArrayLayers(1)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setTiling(vk::ImageTiling::eOptimal)
				.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
				.setSharingMode(vk::SharingMode::eExclusive)
				.setInitialLayout(vk::ImageLayout::eUndefined);

			vk::Image vkImage = device.createImage(imageInfo);

			// Allocate memory
			vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(vkImage);
			vk::PhysicalDeviceMemoryProperties memProperties = VulkanContext::GetPhysicalDevice().getMemoryProperties();

			uint32_t memoryTypeIndex = UINT32_MAX;
			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((memRequirements.memoryTypeBits & (1 << i)) &&
					(memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal))
				{
					memoryTypeIndex = i;
					break;
				}
			}

			if (memoryTypeIndex == UINT32_MAX)
			{
				device.destroyImage(vkImage);
				Logger::LogError("Failed to find suitable memory type for image");
				return nullptr;
			}

			vk::MemoryAllocateInfo allocInfo;
			allocInfo.setAllocationSize(memRequirements.size)
				.setMemoryTypeIndex(memoryTypeIndex);

			vk::DeviceMemory imageMemory = device.allocateMemory(allocInfo);
			device.bindImageMemory(vkImage, imageMemory, 0);

			// Create staging buffer
			const size_t imageSize = width * height * 4;
			VulkanHostBuffer* stagingBuffer = CreateHostBuffer(imageSize, HostBufferUsage::TransferSrc);
			if (!stagingBuffer)
			{
				device.destroyImage(vkImage);
				device.freeMemory(imageMemory);
				Logger::LogError("Failed to create staging buffer for image");
				return nullptr;
			}

			// Copy image data to staging buffer
			std::memcpy(stagingBuffer->mappedAddr, image.GetBytesData(), imageSize);

			// Create command buffer for transfer
			vk::CommandPoolCreateInfo poolInfo;
			poolInfo.setQueueFamilyIndex(VulkanContext::GetGraphicQueueIndex());
			vk::CommandPool commandPool = device.createCommandPool(poolInfo);

			vk::CommandBufferAllocateInfo allocInfo2;
			allocInfo2.setCommandPool(commandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1);

			vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo2)[0];

			// Begin command buffer
			vk::CommandBufferBeginInfo beginInfo;
			beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
			commandBuffer.begin(beginInfo);

			// Transition image layout to transfer dst
			vk::ImageMemoryBarrier barrier;
			barrier.setOldLayout(vk::ImageLayout::eUndefined)
				.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setImage(vkImage)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
				.setSrcAccessMask(vk::AccessFlagBits::eNone)
				.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eTransfer,
				{}, {}, {}, barrier);

			// Copy buffer to image
			vk::BufferImageCopy region;
			region.setBufferOffset(0)
				.setBufferRowLength(0)
				.setBufferImageHeight(0)
				.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
				.setImageOffset(vk::Offset3D(0, 0, 0))
				.setImageExtent(vk::Extent3D(width, height, 1));

			commandBuffer.copyBufferToImage(stagingBuffer->buffer, vkImage, vk::ImageLayout::eTransferDstOptimal, region);

			// Transition image layout to shader read only
			barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
				.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eFragmentShader,
				{}, {}, {}, barrier);

			commandBuffer.end();

			// Submit command buffer
			vk::SubmitInfo submitInfo;
			submitInfo.setCommandBuffers(commandBuffer);
			VulkanContext::GetGraphicQueue().submit(submitInfo);
			VulkanContext::GetGraphicQueue().waitIdle();

			// Cleanup
			device.destroyCommandPool(commandPool);
			stagingBuffer->MarkDelete();

			// Create image view
			vk::ImageViewCreateInfo viewInfo;
			viewInfo.setImage(vkImage)
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(format)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

			vk::ImageView imageView = device.createImageView(viewInfo);

			VulkanImage* pImageRaw = new VulkanImage(width, height, format, vkImage, imageMemory, imageView);
			_resources.push_back(ResourcePtr(pImageRaw, &DeleteVulkanResource<VulkanImage>));

			return pImageRaw;
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create vulkan image: {}", e.what());
			return nullptr;
		}
	}

	VulkanSampler* VulkanResourceManager::CreateSampler()
	{
		const auto device = VulkanContext::GetDevice();

		try
		{
			vk::SamplerCreateInfo samplerInfo;
			samplerInfo.setMagFilter(vk::Filter::eLinear)
				.setMinFilter(vk::Filter::eLinear)
				.setMipmapMode(vk::SamplerMipmapMode::eLinear)
				.setAddressModeU(vk::SamplerAddressMode::eRepeat)
				.setAddressModeV(vk::SamplerAddressMode::eRepeat)
				.setAddressModeW(vk::SamplerAddressMode::eRepeat)
				.setMipLodBias(0.0f)
				.setAnisotropyEnable(VK_TRUE)
				.setMaxAnisotropy(16.0f)
				.setCompareEnable(VK_FALSE)
				.setCompareOp(vk::CompareOp::eAlways)
				.setMinLod(0.0f)
				.setMaxLod(0.0f)
				.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
				.setUnnormalizedCoordinates(VK_FALSE);

			vk::Sampler sampler = device.createSampler(samplerInfo);

			VulkanSampler* pSamplerRaw = new VulkanSampler(sampler);
			_resources.push_back(ResourcePtr(pSamplerRaw, &DeleteVulkanResource<VulkanSampler>));

			return pSamplerRaw;
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create vulkan sampler: {}", e.what());
			return nullptr;
		}
	}

} // namespace Ailurus