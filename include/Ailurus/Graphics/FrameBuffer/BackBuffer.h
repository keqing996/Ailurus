#pragma once

#include <vulkan/vulkan.hpp>
#include "../SwapChain/SwapChain.h"
#include "../RenderPass/RenderPass.h"

namespace Ailurus
{
    class Renderer;

    class BackBuffer
    {
    public:
        BackBuffer(const Renderer* pRenderer, const SwapChain* pSwapChain, const RenderPass* pRenderPass);
        ~BackBuffer();

        const std::vector<vk::Framebuffer>& GetBackBuffers();

    protected:
        const Renderer* _pRenderer;
        std::vector<vk::Framebuffer> _vkFrameBuffer;

    };
}