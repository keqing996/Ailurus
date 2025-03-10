#pragma once

#include <functional>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class Renderer;

    class VulkanContext
    {
        using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
        using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
        using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;

        VulkanContext() = default;

    public:
        static bool enableValidation;

    public:
        static void SetCallbackGetWindowInstanceExtension(const GetWindowInstanceExtension& f);
        static void SetCallbackWindowCreateSurfaceCallback(const WindowCreateSurfaceCallback& f);
        static void SetCallbackWindowDestroySurfaceCallback(const WindowDestroySurfaceCallback& f);

        static void Init();
        static void Destroy();
        static bool Register(const Renderer* pRenderer, vk::SurfaceKHR* outSurface);
        static void Unregister(const Renderer* pRenderer);

        static vk::Device GetDevice();
        static uint32_t GetGraphicQueueIndex();
        static vk::Queue GetGraphicQueue();
        static uint32_t GetComputeQueueIndex();
        static vk::Queue GetComputeQueue();

    private:
        static bool InitVulkan();
        static void DestroyVulkan();
        static void CreateInstance();
        static void CreatDebugUtilsMessenger();
        static void ChoosePhysicsDevice();
        static bool CreateLogicalDevice();

    private:
        static GetWindowInstanceExtension _getWindowRequiredExtension;
        static WindowCreateSurfaceCallback _createSurface;
        static WindowDestroySurfaceCallback _destroySurface;

        static vk::Instance _vkInstance;
        static vk::DebugUtilsMessengerEXT _vkDebugUtilsMessenger;
        static vk::PhysicalDevice _vkPhysicalDevice;
        static vk::SurfaceKHR _vkSurface;
        static vk::Device _vkDevice;
        static uint32_t _graphicQueueIndex;
        static uint32_t _computeQueueIndex;
        static vk::Queue _vkGraphicQueue;
        static vk::Queue _vkComputeQueue;

    };
}