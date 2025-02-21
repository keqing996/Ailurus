#pragma once

#include <vulkan/vulkan.hpp>
#include "RenderPassType.h"
#include "Ailurus/Graphics/Renderer.h"

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