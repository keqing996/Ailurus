#include <string_view>
#include "Ailurus/Renderer/Utility/VulkanUtil.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    VkResult VulkanUtil::EnumerateInstanceLayerProperties(std::vector<VkLayerProperties>& layers)
    {
        uint32_t layersCount = 0;

        VkResult result = ::vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
        if (result != VK_SUCCESS)
            return result;

        layers.resize(layersCount);

        return ::vkEnumerateInstanceLayerProperties(&layersCount, layers.data());
    }

    VkResult VulkanUtil::EnumeratePhysicalDevices(VkInstance instance, std::vector<VkPhysicalDevice>& devices)
    {
        uint32_t deviceCount = 0;

        VkResult result = ::vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (result != VK_SUCCESS)
            return result;

        devices.resize(deviceCount);

        return ::vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    }

    VkResult VulkanUtil::EnumerateDeviceExtensionProperties(VkPhysicalDevice device,
                                                            std::vector<VkExtensionProperties>& extensions)
    {
        uint32_t extensionCount = 0;

        VkResult result = ::vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        if (result != VK_SUCCESS)
            return result;

        extensions.resize(extensionCount);

        return ::vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());
    }

    bool VulkanUtil::IsPhysicalDeviceSupportSwapChain(VkPhysicalDevice device)
    {
        std::vector<VkExtensionProperties> extensions;
        if (EnumerateDeviceExtensionProperties(device, extensions) != VK_SUCCESS)
            return false;

        for (VkExtensionProperties& extension: extensions)
        {
            if (std::string_view(extension.extensionName) == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                return true;
        }

        return false;
    }

    void VulkanUtil::GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice device,
                                                            std::vector<VkQueueFamilyProperties>& properties)
    {
        uint32_t propertiesCount = 0;

        ::vkGetPhysicalDeviceQueueFamilyProperties(device, &propertiesCount, nullptr);

        properties.resize(propertiesCount);

        ::vkGetPhysicalDeviceQueueFamilyProperties(device, &propertiesCount, properties.data());
    }

    VkResult VulkanUtil::GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice device, VkSurfaceKHR surface,
                                                            std::vector<VkSurfaceFormatKHR>& formats)
    {
        uint32_t formatsCount = 0;

        VkResult result = ::vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, nullptr);
        if (result != VK_SUCCESS)
            return result;

        formats.resize(formatsCount);

        return ::vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, formats.data());
    }

    VkResult VulkanUtil::GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice device, VkSurfaceKHR surface,
                                                                 std::vector<VkPresentModeKHR>& modes)
    {
        uint32_t modesCount = 0;

        VkResult result = ::vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modesCount, nullptr);
        if (result != VK_SUCCESS)
            return result;

        modes.resize(modesCount);

        return ::vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modesCount, modes.data());
    }

    VkResult VulkanUtil::GetSwapChainImagesKHR(VkDevice logicDevice, VkSwapchainKHR swapChain,
                                               std::vector<VkImage>& images)
    {
        uint32_t imagesCount = 0;

        VkResult result = ::vkGetSwapchainImagesKHR(logicDevice, swapChain, &imagesCount, nullptr);
        if (result != VK_SUCCESS)
            return result;

        images.resize(imagesCount);

        return ::vkGetSwapchainImagesKHR(logicDevice, swapChain, &imagesCount, images.data());
    }

    VkBool32 VulkanUtil::DebugReportExtCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, std::uint64_t,
                                                std::size_t, std::int32_t, const char*, const char* pMessage, void*)
    {
        static const char* Tag = "Vulkan";
        Logger::LogError(Tag, pMessage);
        return VK_FALSE;
    }
}
