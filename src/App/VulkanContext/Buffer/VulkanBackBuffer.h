#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class Renderer;
    class RenderPass;
    class SwapChain;

    class VulkanBackBuffer
    {
    public:
        explicit VulkanBackBuffer(const RenderPass* pRenderPass);
        ~VulkanBackBuffer();

        const std::vector<vk::Framebuffer>& GetBackBuffers();

    protected:
        std::vector<vk::Framebuffer> _vkFrameBuffer;
    };
}