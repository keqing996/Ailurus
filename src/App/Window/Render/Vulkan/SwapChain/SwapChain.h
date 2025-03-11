#pragma once

#include <vulkan/vulkan.hpp>
#include "SwapChainConfig.h"

namespace Ailurus
{
    class Render;

    class SwapChain
    {
    public:
        SwapChain(const Render* pRenderer, int windowWidth, int windowHeight);
        ~SwapChain();

    public:
        const SwapChainConfig& GetSwapChainConfig() const;
        const vk::SwapchainKHR& GetSwapChain() const;
        const std::vector<vk::Image>& GetImages() const;
        const std::vector<vk::ImageView>& GetImageViews() const;
    private:
        const Render* _pRenderer;

        SwapChainConfig _swapChainConfig{};
        vk::SwapchainKHR _vkSwapChain = nullptr;
        std::vector<vk::Image> _vkSwapChainImages;
        std::vector<vk::ImageView> _vkSwapChainImageViews;
    };
}