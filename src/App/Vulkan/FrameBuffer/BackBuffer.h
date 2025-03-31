#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class Renderer;
    class RenderPass;
    class SwapChain;

    class BackBuffer
    {
    public:
        explicit BackBuffer(const RenderPass* pRenderPass);
        ~BackBuffer();

        const std::vector<vk::Framebuffer>& GetBackBuffers();

    protected:
        std::vector<vk::Framebuffer> _vkFrameBuffer;

    };
}