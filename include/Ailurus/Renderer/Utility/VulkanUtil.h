#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace Ailurus
{
    class VulkanUtil
    {
    public:
        VulkanUtil() = delete;

    public:
        static VkResult
        EnumerateInstanceLayerProperties(
            std::vector<VkLayerProperties>& layers);

        static VkResult
        EnumeratePhysicalDevices(
            VkInstance instance,
            std::vector<VkPhysicalDevice>& devices);

        static VkResult
        EnumerateDeviceExtensionProperties(
            VkPhysicalDevice device,
            std::vector<VkExtensionProperties>& extensions);

        static bool
        IsPhysicalDeviceSupportSwapChain(
            VkPhysicalDevice device);

        static void
        GetPhysicalDeviceQueueFamilyProperties(
            VkPhysicalDevice device,
            std::vector<VkQueueFamilyProperties>& properties);

        static VkResult
        GetPhysicalDeviceSurfaceFormatsKHR(
            VkPhysicalDevice device,
            VkSurfaceKHR surface,
            std::vector<VkSurfaceFormatKHR>& formats);

        static VkResult
        GetPhysicalDeviceSurfacePresentModesKHR(
            VkPhysicalDevice device,
            VkSurfaceKHR surface,
            std::vector<VkPresentModeKHR>& modes);

        static VkResult
        GetSwapChainImagesKHR(
            VkDevice logicDevice,
            VkSwapchainKHR swapChain,
            std::vector<VkImage>& images);

        static VKAPI_ATTR VkBool32 VKAPI_CALL
        DebugReportExtCallback(
            VkDebugReportFlagsEXT,
            VkDebugReportObjectTypeEXT,
            std::uint64_t,
            std::size_t,
            std::int32_t,
            const char*,
            const char* pMessage,
            void*);
    };
}
