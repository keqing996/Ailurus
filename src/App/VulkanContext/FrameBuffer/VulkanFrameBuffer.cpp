#include "VulkanFrameBuffer.h"
#include "VulkanContext/VulkanContext.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	vk::Framebuffer VulkanFrameBuffer::GetBuffer() const
	{
		return _vkFramebuffer;
	}

	VulkanFrameBuffer::VulkanFrameBuffer(vk::Framebuffer framebuffer)
		: _vkFramebuffer(framebuffer)
	{
	}

	VulkanFrameBuffer::~VulkanFrameBuffer()
	{
		try
		{
			VulkanContext::GetDevice().destroyFramebuffer(_vkFramebuffer);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to destroy framebuffer: {}", e.what());
		}
	}
} // namespace Ailurus