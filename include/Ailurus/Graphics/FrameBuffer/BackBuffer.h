#pragma once

#include <vulkan/vulkan.hpp>
#include "../Context/VulkanContext.h"
#include "../SwapChain/SwapChain.h"
#include "../RenderPass/RenderPass.h"

namespace Ailurus
{
    class BackBuffer
    {
    public:
        BackBuffer(const VulkanContext* pContext, const SwapChain* pSwapChain, const RenderPass* pRenderPass);
        ~BackBuffer();

        const std::vector<vk::Framebuffer>& GetBackBuffers();

    protected:
        const VulkanContext* _pContext;
        std::vector<vk::Framebuffer> _vkFrameBuffer;

    };
}