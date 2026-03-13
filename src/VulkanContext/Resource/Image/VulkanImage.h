#pragma once

#include <cstdint>
#include "VulkanContext/VulkanPch.h"
#include "VulkanContext/Resource/VulkanResource.h"

namespace Ailurus
{
	class Image;

	// Configuration for creating VulkanImage with flexible parameters (cubemap, HDR, mipmap)
	struct VulkanImageCreateConfig
	{
		uint32_t width = 0;
		uint32_t height = 0;
		vk::Format format = vk::Format::eR8G8B8A8Srgb;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		vk::ImageCreateFlags flags = {};                 // eCubeCompatible for cubemap
		vk::ImageViewType viewType = vk::ImageViewType::e2D;
		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor;
	};

	class VulkanImage : public VulkanResource
	{
		friend class VulkanResourceManager;

	public:
		VulkanImage(uint32_t width, uint32_t height, vk::Format format,
			vk::Image image, vk::DeviceMemory memory, vk::ImageView imageView,
			uint32_t mipLevels = 1, uint32_t arrayLayers = 1,
			vk::ImageViewType viewType = vk::ImageViewType::e2D);

	public:
		auto GetWidth() const -> uint32_t { return _width; }
		auto GetHeight() const -> uint32_t { return _height; }
		auto GetFormat() const -> vk::Format { return _format; }
		auto GetImage() const -> vk::Image { return _image; }
		auto GetImageView() const -> vk::ImageView { return _imageView; }
		auto GetMemory() const -> vk::DeviceMemory { return _memory; }
		auto GetMipLevels() const -> uint32_t { return _mipLevels; }
		auto GetArrayLayers() const -> uint32_t { return _arrayLayers; }
		auto GetViewType() const -> vk::ImageViewType { return _viewType; }

	private:
		uint32_t _width;
		uint32_t _height;
		vk::Format _format;
		vk::Image _image;
		vk::DeviceMemory _memory;
		vk::ImageView _imageView;
		uint32_t _mipLevels = 1;
		uint32_t _arrayLayers = 1;
		vk::ImageViewType _viewType = vk::ImageViewType::e2D;

	private:
		friend class VulkanResourceManager;
		static VulkanResourcePtr Create(const Image& image);
		static VulkanResourcePtr CreateFromConfig(const VulkanImageCreateConfig& config,
			const void* pixelData, size_t dataSize);
	};
} // namespace Ailurus
