#include "VulkanImage.h"
#include "VulkanContext/VulkanContext.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
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

	VulkanImage::~VulkanImage()
	{
		auto device = VulkanContext::GetDevice();

		if (_imageView)
		{
			device.destroyImageView(_imageView);
			_imageView = nullptr;
		}

		if (_image)
		{
			device.destroyImage(_image);
			_image = nullptr;
		}

		if (_memory)
		{
			device.freeMemory(_memory);
			_memory = nullptr;
		}
	}
} // namespace Ailurus
