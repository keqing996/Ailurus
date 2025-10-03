#include "VulkanImage.h"
#include "VulkanContext/VulkanContext.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Utility/Image.h"
#include "VulkanContext/Resource/VulkanResourceManager.h"
#include "VulkanContext/Resource/DataBuffer/VulkanHostBuffer.h"

namespace Ailurus
{
	static void ImageDeleter(VulkanResource* pResource)
	{
		auto ptr = static_cast<VulkanImage*>(pResource);
		auto device = VulkanContext::GetDevice();

		auto imageView = ptr->GetImageView();
		if (imageView)
			device.destroyImageView(imageView);

		auto image = ptr->GetImage();
		if (image)
			device.destroyImage(image);

		auto memory = ptr->GetMemory();
		if (memory)
			device.freeMemory(memory);
	}

	VulkanImage::VulkanImage(uint32_t width, uint32_t height, vk::Format format,
		vk::Image image, vk::DeviceMemory memory, vk::ImageView imageView)
		: _width(width)
		, _height(height)
		, _format(format)
		, _image(image)
		, _memory(memory)
		, _imageView(imageView)
	{
	}

	VulkanResourcePtr VulkanImage::Create(const Image& image)
	{
		const auto device = VulkanContext::GetDevice();
		const auto [width, height] = image.GetPixelSize();
		const vk::Format format = vk::Format::eR8G8B8A8Srgb;
		const auto pVulkanResManager = VulkanContext::GetResourceManager();

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
			VulkanHostBuffer* stagingBuffer = pVulkanResManager->CreateHostBuffer(imageSize, HostBufferUsage::TransferSrc);
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
			return VulkanResourcePtr(pImageRaw, &ImageDeleter);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create vulkan image: {}", e.what());
			return nullptr;
		}
	}
} // namespace Ailurus
