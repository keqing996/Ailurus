#include "VulkanFrameBuffer.h"
#include "VulkanContext/VulkanContext.h"

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
        VulkanContext::GetDevice().destroyFramebuffer(_vkFramebuffer);
	}
} // namespace Ailurus