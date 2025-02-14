#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>
#include "SwapChainConfig.h"
#include "../Context/VulkanContext.h"

namespace Ailurus
{
    class SwapChain
    {
    public:
        ~SwapChain();

    public:
        static std::unique_ptr<SwapChain> Create(const VulkanContext* pContext, int windowWidth, int windowHeight);

        const SwapChainConfig& GetSwapChainConfig() const;
        const vk::SwapchainKHR& GetSwapChain() const;
        const std::vector<vk::Image>& GetImages() const;
        const std::vector<vk::ImageView>& GetImageViews() const;

    private:
        SwapChain(const VulkanContext* pContext, SwapChainConfig config);

    private:
        const VulkanContext* _pContext;

        SwapChainConfig _swapChainConfig{};
        vk::SwapchainKHR _vkSwapChain = nullptr;
        std::vector<vk::Image> _vkSwapChainImages;
        std::vector<vk::ImageView> _vkSwapChainImageViews;
    };
}