#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>
#include "../SwapChain/SwapChain.h"

namespace Ailurus
{
    class Renderer;

    class RenderPass
    {
    public:
        RenderPass(const Renderer* pRenderer, const SwapChain* pSwapChain);
        ~RenderPass();

    public:
        vk::RenderPass GetRenderPass() const;

    private:
        const Renderer* _pRenderer;
        vk::RenderPass _vkRenderPass;
    };
}