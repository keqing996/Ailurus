#pragma once

#include "VulkanContext/VulkanPch.h"
#include <cstdint>
#include "VulkanResource.h"

namespace Ailurus
{
	class VulkanImage : public VulkanResource
	{
		friend class VulkanResourceManager;

	public:
		VulkanImage(uint32_t width, uint32_t height, vk::Format format,
			vk::Image image, vk::DeviceMemory memory, vk::ImageView imageView);
		~VulkanImage() override;

	public:
		auto GetWidth() const -> uint32_t { return _width; }
		auto GetHeight() const -> uint32_t { return _height; }
		auto GetFormat() const -> vk::Format { return _format; }
		auto GetImage() const -> vk::Image { return _image; }
		auto GetImageView() const -> vk::ImageView { return _imageView; }

	private:
		uint32_t _width;
		uint32_t _height;
		vk::Format _format;
		vk::Image _image;
		vk::DeviceMemory _memory;
		vk::ImageView _imageView;
	};
} // namespace Ailurus
