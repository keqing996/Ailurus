#include "Ailurus/Renderer/Renderer.h"

#include <algorithm>

#include "SDL3/SDL_vulkan.h"

namespace Ailurus
{
    Renderer::Renderer(Window* pWindow, bool enableDebugReport, bool enableValidationLayer)
        : _pWindow(pWindow)
    {
        if (vulkanAvailable)
            VulkanInitInstance(enableDebugReport, enableValidationLayer);

        if (vulkanAvailable && enableDebugReport)
            VulkanInitDebugReportCallbackExt();

        if (vulkanAvailable)
            VulkanInitSurface();

        if (vulkanAvailable)
            VulkanInitPhysicsDevice();

        if (vulkanAvailable)
            VulkanInitDepthFormat();

        if (vulkanAvailable)
            VulkanInitLogicDevice();

        if (vulkanAvailable)
            VulkanInitSwapChainFormat();

        if (vulkanAvailable)
            VulkanInitSwapChain();

        if (vulkanAvailable)
            VulkanInitSwapChainImage();
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::VulkanInitInstance(bool enableDebugReport, bool enableValidation)
    {
        std::vector<VkLayerProperties> layers;
        if (VulkanUtil::EnumerateInstanceLayerProperties(layers) != VK_SUCCESS)
        {
            vulkanAvailable = false;
            return;
        }

        // Validation layers
        std::vector<const char*> validationLayers;
        if (enableValidation)
        {
            for (const VkLayerProperties& layer : layers)
            {
                if (std::string_view(layer.layerName) == "VK_LAYER_LUNARG_standard_validation")
                    validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
                else if (std::string_view(layer.layerName) == "VK_LAYER_LUNARG_monitor")
                    validationLayers.push_back("VK_LAYER_LUNARG_monitor");
            }
        }

        // Extensions
        std::vector<const char*> requiredExtensions;

        if (enableDebugReport)
            requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

        uint32_t sdlExtensionCount = 0;
        const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
        for (uint32_t n = 0; n < sdlExtensionCount; n++)
            requiredExtensions.push_back(sdl_extensions[n]);

        // Create instance
        VkApplicationInfo applicationInfo  = VkApplicationInfo();
        applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName   = "Ailurus";
        applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.pEngineName        = "No Engine";
        applicationInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.apiVersion         = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceCreateInfo    = VkInstanceCreateInfo();
        instanceCreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo        = &applicationInfo;
        instanceCreateInfo.enabledLayerCount       = static_cast<std::uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames     = validationLayers.data();
        instanceCreateInfo.enabledExtensionCount   = static_cast<std::uint32_t>(requiredExtensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

        if (::vkCreateInstance(&instanceCreateInfo, nullptr, &_vkInstance) != VK_SUCCESS)
        {
            vulkanAvailable = false;
            return;
        }
    }

    void Renderer::VulkanInitDebugReportCallbackExt()
    {
        VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = VkDebugReportCallbackCreateInfoEXT();
        debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT
            | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
            | VK_DEBUG_REPORT_ERROR_BIT_EXT;
        debugReportCallbackCreateInfo.pfnCallback = VulkanUtil::DebugReportExtCallback;

        // Create the debug callback
        if (::vkCreateDebugReportCallbackEXT(_vkInstance, &debugReportCallbackCreateInfo, nullptr, &_vkDebugReportCallbackExt) != VK_SUCCESS)
        {
            vulkanAvailable = false;
            return;
        }
    }

    void Renderer::VulkanInitSurface()
    {
        if (::SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(_pWindow->GetSDLWindowPtr()),
            _vkInstance, nullptr, &_vkSurface) == VK_SUCCESS)
        {
            vulkanAvailable = false;
            return;
        }
    }

    void Renderer::VulkanInitPhysicsDevice()
    {
        std::vector<VkPhysicalDevice> devices;
        if (VulkanUtil::EnumeratePhysicalDevices(_vkInstance, devices) != VK_SUCCESS)
        {
            vulkanAvailable = false;
            return;
        }

        VkPhysicalDeviceType lastFoundDeviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
        for (VkPhysicalDevice singleDevice : devices)
        {
            if (!VulkanUtil::IsPhysicalDeviceSupportSwapChain(singleDevice))
                continue;

            VkPhysicalDeviceProperties deviceProperties;
            ::vkGetPhysicalDeviceProperties(singleDevice, &deviceProperties);

            // Discrete gpu found, no need to continue.
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                _vkPhysicalDevice = singleDevice;
                break;
            }

            // Integrated gpu found, record it and keep finding discrete gpu.
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                _vkPhysicalDevice = singleDevice;
                lastFoundDeviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
            }

            // Cpu found, record it only if that no gpu was found.
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU
                && lastFoundDeviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                _vkPhysicalDevice = singleDevice;
                lastFoundDeviceType = VK_PHYSICAL_DEVICE_TYPE_CPU;
            }
        }

        if (!_vkPhysicalDevice)
        {
            vulkanAvailable = false;
            return;
        }

    }

    void Renderer::VulkanInitDepthFormat()
    {
        VkFormatProperties formatProperties = VkFormatProperties();

        ::vkGetPhysicalDeviceFormatProperties(_vkPhysicalDevice, VK_FORMAT_D24_UNORM_S8_UINT, &formatProperties);

        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            _vkDepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        }
        else
        {
            ::vkGetPhysicalDeviceFormatProperties(_vkPhysicalDevice, VK_FORMAT_D32_SFLOAT_S8_UINT, &formatProperties);

            if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                _vkDepthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
            }
            else
            {
                ::vkGetPhysicalDeviceFormatProperties(_vkPhysicalDevice, VK_FORMAT_D32_SFLOAT, &formatProperties);

                if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                {
                    _vkDepthFormat = VK_FORMAT_D32_SFLOAT;
                }
                else
                {
                    vulkanAvailable = false;
                    return;
                }
            }
        }
    }

    void Renderer::VulkanInitLogicDevice()
    {
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
        VulkanUtil::GetPhysicalDeviceQueueFamilyProperties(_vkPhysicalDevice, queueFamilyProperties);

        for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
        {
            VkBool32 surfaceSupported = VK_FALSE;
            ::vkGetPhysicalDeviceSurfaceSupportKHR(_vkPhysicalDevice, static_cast<uint32_t>(i), _vkSurface, &surfaceSupported);

            if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && surfaceSupported == VK_TRUE)
            {
                _vkQueueFamilyIndex = static_cast<uint32_t>(i);
                break;
            }
        }

        // Gpu only support compute, do not support graphic.
        if (!_vkQueueFamilyIndex.has_value())
        {
            vulkanAvailable = false;
            return;
        }

        const float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo deviceQueueCreateInfo = VkDeviceQueueCreateInfo();
        deviceQueueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueCount              = 1;
        deviceQueueCreateInfo.queueFamilyIndex        = _vkQueueFamilyIndex.value();
        deviceQueueCreateInfo.pQueuePriorities        = &queuePriority;

        // Swap chain is required
        const char* extensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        // Anisotropic filtering
        VkPhysicalDeviceFeatures physicalDeviceFeatures = VkPhysicalDeviceFeatures();
        physicalDeviceFeatures.samplerAnisotropy        = VK_TRUE;

        VkDeviceCreateInfo deviceCreateInfo      = VkDeviceCreateInfo();
        deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.enabledExtensionCount   = 1;
        deviceCreateInfo.ppEnabledExtensionNames = extensions;
        deviceCreateInfo.queueCreateInfoCount    = 1;
        deviceCreateInfo.pQueueCreateInfos       = &deviceQueueCreateInfo;
        deviceCreateInfo.pEnabledFeatures        = &physicalDeviceFeatures;

        if (::vkCreateDevice(_vkPhysicalDevice, &deviceCreateInfo, nullptr, &_vkLogicDevice) != VK_SUCCESS)
        {
            vulkanAvailable = false;
            return;
        }

        ::vkGetDeviceQueue(_vkLogicDevice, *_vkQueueFamilyIndex, 0, &_vkQueue);
    }

    void Renderer::VulkanInitSwapChainFormat()
    {
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        if (VulkanUtil::GetPhysicalDeviceSurfaceFormatsKHR(_vkPhysicalDevice, _vkSurface, surfaceFormats) != VK_SUCCESS)
        {
            vulkanAvailable = false;
            return;
        }

        if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
        {
            _vkSwapChainFormat.format     = VK_FORMAT_B8G8R8A8_UNORM;
            _vkSwapChainFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
        else if (!surfaceFormats.empty())
        {
            for (const VkSurfaceFormatKHR& surfaceFormat : surfaceFormats)
            {
                if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    _vkSwapChainFormat.format     = VK_FORMAT_B8G8R8A8_UNORM;
                    _vkSwapChainFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                    break;
                }
            }

            if (_vkSwapChainFormat.format == VK_FORMAT_UNDEFINED)
                _vkSwapChainFormat = surfaceFormats[0];
        }
        else
        {
            vulkanAvailable = false;
            return;
        }

    }

    void Renderer::VulkanInitSwapChain()
    {
        std::vector<VkPresentModeKHR> presentModes;
        if (VulkanUtil::GetPhysicalDeviceSurfacePresentModesKHR(_vkPhysicalDevice, _vkSurface, presentModes) != VK_SUCCESS)
        {
            vulkanAvailable = false;
            return;
        }

        // mailbox > fifo
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (const VkPresentModeKHR& mode : presentModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = mode;
                break;
            }
        }

        // Swap chain image size
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        if (::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_vkPhysicalDevice, _vkSurface, &surfaceCapabilities) != VK_SUCCESS)
        {
            vulkanAvailable = false;
            return;
        }

        Vector2i windowSize = _pWindow->GetSize();
        _vkSwapChainExtent.width  = std::clamp(static_cast<uint32_t>(windowSize.x()),
                                           surfaceCapabilities.minImageExtent.width,
                                           surfaceCapabilities.maxImageExtent.width);
        _vkSwapChainExtent.height = std::clamp(static_cast<uint32_t>(windowSize.y()),
                                            surfaceCapabilities.minImageExtent.height,
                                            surfaceCapabilities.maxImageExtent.height);

        // Default 2 images for swap chain
        const auto imageCount = std::clamp(2u, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

        VkSwapchainCreateInfoKHR swapChainCreateInfo = VkSwapchainCreateInfoKHR();
        swapChainCreateInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface                  = _vkSurface;
        swapChainCreateInfo.minImageCount            = imageCount;
        swapChainCreateInfo.imageFormat              = _vkSwapChainFormat.format;
        swapChainCreateInfo.imageColorSpace          = _vkSwapChainFormat.colorSpace;
        swapChainCreateInfo.imageExtent              = _vkSwapChainExtent;
        swapChainCreateInfo.imageArrayLayers         = 1;
        swapChainCreateInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapChainCreateInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.preTransform             = surfaceCapabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.presentMode              = presentMode;
        swapChainCreateInfo.clipped                  = VK_TRUE;
        swapChainCreateInfo.oldSwapchain             = VK_NULL_HANDLE;

        if (::vkCreateSwapchainKHR(_vkLogicDevice, &swapChainCreateInfo, nullptr, &_vkSwapChain) != VK_SUCCESS)
        {
            vulkanAvailable = false;
            return;
        }

    }
    void Renderer::VulkanInitSwapChainImage()
    {
        if (VulkanUtil::GetSwapChainImagesKHR(_vkLogicDevice, _vkSwapChain, _vkSwapChainImages) != VK_SUCCESS)
        {
            vulkanAvailable = false;
            return;
        }

        VkImageViewCreateInfo imageViewCreateInfo           = VkImageViewCreateInfo();
        imageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format                          = _vkSwapChainFormat.format;
        imageViewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
        imageViewCreateInfo.subresourceRange.levelCount     = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = 1;

        _vkSwapChainImageViews.resize(_vkSwapChainImages.size());
        for (std::size_t i = 0; i < _vkSwapChainImages.size(); ++i)
        {
            imageViewCreateInfo.image = _vkSwapChainImages[i];

            if (::vkCreateImageView(_vkLogicDevice, &imageViewCreateInfo, nullptr, &_vkSwapChainImageViews[i]) != VK_SUCCESS)
            {
                vulkanAvailable = false;
                return;
            }
        }

    }
}