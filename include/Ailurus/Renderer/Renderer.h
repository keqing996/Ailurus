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

        struct SwapChainConfig
        {
            vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
            vk::SurfaceFormatKHR surfaceFormat = vk::Format::eUndefined;
            vk::Extent2D extent;
            uint32_t imageCount;
        };

        void CreateInstance(bool enableValidation);
        void CreatDebugReportCallbackExt();
        void CreateSurface(const WindowCreateSurfaceCallback& createSurface);
        void ChoosePhysicsDevice();
        void CreateLogicDevice();
        void CreateSwapChain();

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
        SwapChainConfig _swapChainConfig;
        vk::SwapchainKHR _vkSwapChain = nullptr;
        std::vector<vk::Image>        _vkSwapChainImages;
        std::vector<vk::ImageView>    _vkSwapChainImageViews;

    };
}