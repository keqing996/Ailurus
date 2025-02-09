#pragma once

#include <optional>
#include <vulkan/vulkan.h>
#include "Utility/VulkanUtil.h"
#include "../Window/Window.h"

namespace Ailurus
{
    class Renderer
    {
    public:
        Renderer(Window* pWindow, bool enableValidationLayer);
        ~Renderer();

    private:
        void VulkanInitInstance();
        void VulkanInitDebugReportCallbackExt();
        void VulkanInitSurface();
        void VulkanInitDepthFormat();
        void VulkanInitLogicDevice();
        void VulkanInitSwapChainFormat();
        void VulkanInitSwapChain();
        void VulkanInitSwapChainImage();

    private:
        Window* _pWindow;
        bool vulkanAvailable = true;
        VkInstance                  _vkInstance = nullptr;
        VkDebugReportCallbackEXT    _vkDebugReportCallbackExt = nullptr;
        VkSurfaceKHR                _vkSurface = nullptr;
        VkPhysicalDevice            _vkPhysicalDevice = nullptr;
        VkFormat                    _vkDepthFormat = VK_FORMAT_UNDEFINED;
        std::optional<uint32_t>     _vkQueueFamilyIndex;
        VkDevice                    _vkLogicDevice = nullptr;
        VkQueue                     _vkQueue = nullptr;
        VkSurfaceFormatKHR          _vkSwapChainFormat{};
        VkExtent2D                  _vkSwapChainExtent{};
        VkSwapchainKHR              _vkSwapChain{};
        std::vector<VkImage>        _vkSwapChainImages;
        std::vector<VkImageView>    _vkSwapChainImageViews;
    };
}