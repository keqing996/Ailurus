#include "RenderTarget.h"
#include "VulkanContext/VulkanContext.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	RenderTarget::RenderTarget(const RenderTargetConfig& config)
		: _config(config)
	{
		CreateImage();
		AllocateMemory();
		CreateImageView();
	}

	RenderTarget::~RenderTarget()
	{
		try
		{
			if (_imageView != nullptr)
			{
				VulkanContext::GetDevice().destroyImageView(_imageView);
				_imageView = nullptr;
			}

			if (_image != nullptr)
			{
				VulkanContext::GetDevice().destroyImage(_image);
				_image = nullptr;
			}

			if (_memory != nullptr)
			{
				VulkanContext::GetDevice().freeMemory(_memory);
				_memory = nullptr;
			}
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to destroy render target: {}", e.what());
		}
	}

	void RenderTarget::CreateImage()
	{
		vk::ImageUsageFlags usage = _config.usage;
		
		// Add transient flag for MSAA optimization if requested
		if (_config.transient && _config.samples != vk::SampleCountFlagBits::e1)
		{
			usage |= vk::ImageUsageFlagBits::eTransientAttachment;
		}

		vk::ImageCreateInfo imageInfo;
		imageInfo.setImageType(vk::ImageType::e2D)
			.setExtent(vk::Extent3D(_config.width, _config.height, 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setFormat(_config.format)
			.setTiling(vk::ImageTiling::eOptimal)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setUsage(usage)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setSamples(_config.samples);

		try
		{
			_image = VulkanContext::GetDevice().createImage(imageInfo);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create render target image: {}", e.what());
		}
	}

	void RenderTarget::AllocateMemory()
	{
		if (_image == nullptr)
			return;

		vk::MemoryRequirements memRequirements = VulkanContext::GetDevice().getImageMemoryRequirements(_image);
		
		vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		
		// For transient attachments, prefer lazily allocated memory if available
		if (_config.transient && _config.samples != vk::SampleCountFlagBits::e1)
		{
			properties |= vk::MemoryPropertyFlagBits::eLazilyAllocated;
			
			// Try to find lazy allocated memory first
			uint32_t memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
			if (memoryTypeIndex == UINT32_MAX)
			{
				// Fallback to device local only
				properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			}
		}

		uint32_t memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
		if (memoryTypeIndex == UINT32_MAX)
		{
			Logger::LogError("Failed to find suitable memory type for render target");
			return;
		}

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(memoryTypeIndex);

		try
		{
			_memory = VulkanContext::GetDevice().allocateMemory(allocInfo);
			VulkanContext::GetDevice().bindImageMemory(_image, _memory, 0);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to allocate render target memory: {}", e.what());
		}
	}

	void RenderTarget::CreateImageView()
	{
		if (_image == nullptr)
			return;

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.setImage(_image)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(_config.format)
			.setSubresourceRange(vk::ImageSubresourceRange()
				.setAspectMask(_config.aspectMask)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1));

		try
		{
			_imageView = VulkanContext::GetDevice().createImageView(viewInfo);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create render target image view: {}", e.what());
		}
	}

	uint32_t RenderTarget::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
	{
		vk::PhysicalDeviceMemoryProperties memProperties = VulkanContext::GetPhysicalDevice().getMemoryProperties();
		
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		
		return UINT32_MAX;
	}

	vk::Image RenderTarget::GetImage() const
	{
		return _image;
	}

	vk::ImageView RenderTarget::GetImageView() const
	{
		return _imageView;
	}

	const RenderTargetConfig& RenderTarget::GetConfig() const
	{
		return _config;
	}

	bool RenderTarget::IsValid() const
	{
		return _image != nullptr && _imageView != nullptr;
	}
} // namespace Ailurus
