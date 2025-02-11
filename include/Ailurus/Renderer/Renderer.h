#pragma once

#include <optional>
#include <functional>
#include <vulkan/vulkan.hpp>
#include "../Math/Vector.hpp"

namespace Ailurus
{
    class Renderer
    {
        using GetWindowSizeCallback = std::function<Vector2i()>;
        using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
        using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
        using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;

    public:
        Renderer(const GetWindowSizeCallback& getWindowSize,
            const GetWindowInstanceExtension& getWindowInstExt,
            const WindowCreateSurfaceCallback& createSurface,
            const WindowDestroySurfaceCallback& destroySurface,
            bool enableValidationLayer);

        ~Renderer();

    private:
        struct QueueIndex
        {
            std::optional<uint32_t> graphicQueueIndex = std::nullopt;
            std::optional<uint32_t> presentQueueIndex = std::nullopt;
        };

        void CreateInstance(bool enableValidation);
        void CreatDebugReportCallbackExt();
        void CreateSurface(const WindowCreateSurfaceCallback& createSurface);
        void ChoosePhysicsDevice();
        void CreateLogicDevice();
        void CreateSwapChain();
        //void VulkanInitDepthFormat();

        //void VulkanInitSwapChainFormat();
        //void VulkanInitSwapChain();
        //void VulkanInitSwapChainImage();

    private:
        GetWindowSizeCallback _getWindowSizeCallback;
        GetWindowInstanceExtension _getWindowInstExtensionsCallback;
        WindowDestroySurfaceCallback _windowDestroySurfaceCallback;

        vk::Instance                  _vkInstance = nullptr;
        vk::DebugReportCallbackEXT     _vkDebugReportCallbackExt = nullptr;
        vk::SurfaceKHR                _vkSurface = nullptr;
        vk::PhysicalDevice            _vkPhysicalDevice = nullptr;
        vk::Device                    _vkLogicDevice = nullptr;
        QueueIndex _queueIndex;
        vk::Queue   _vkGraphicQueue = nullptr;
        vk::Queue   _vkPresentQueue = nullptr;
        vk::SwapchainKHR _vkSwapChain = nullptr;

        //VkFormat                    _vkDepthFormat = VK_FORMAT_UNDEFINED;

        //VkQueue                     _vkQueue = nullptr;
        //VkSurfaceFormatKHR          _vkSwapChainFormat{};
        //VkExtent2D                  _vkSwapChainExtent{};
        //std::vector<VkImage>        _vkSwapChainImages;
        //std::vector<VkImageView>    _vkSwapChainImageViews;
    };
}