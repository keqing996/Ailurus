#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
    class VulkanFrameBufferManager : public NonCopyable, public NonMovable
    {
    public:
        VulkanFrameBufferManager();
        ~VulkanFrameBufferManager();

    public:
        void RecreateBackBuffers();

    private:
        void CreateBackBuffers();
        void DestroyBackBuffers();

    private:
        std::vector<vk::Framebuffer> _vkBackBuffers;
    };
}