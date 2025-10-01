#pragma once

#include "VulkanContext/VulkanPch.h"
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
    class VulkanFrameBuffer: public NonCopyable, public NonMovable
    {
    public:
		VulkanFrameBuffer(vk::Framebuffer framebuffer);
		~VulkanFrameBuffer();

    public:
        vk::Framebuffer GetBuffer() const;

    private:
        vk::Framebuffer _vkFramebuffer;
    };
}