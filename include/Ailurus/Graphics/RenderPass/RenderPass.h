#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>
#include "../Context/VulkanContext.h"
#include "../SwapChain/SwapChain.h"

namespace Ailurus
{
    class RenderPass
    {
    public:
        ~RenderPass();

    public:
        static std::unique_ptr<RenderPass> Create(const VulkanContext* pContext, const SwapChain* pSwapChain);

        vk::RenderPass GetRenderPass() const;

    private:
        RenderPass(const VulkanContext* pContext, const SwapChain* pSwapChain);

    private:
        const VulkanContext* _pContext;
        vk::RenderPass _vkRenderPass;
    };
}