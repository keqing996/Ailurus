#include <algorithm>
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Renderer/Renderer.h"
#include "Ailurus/Renderer/Utility/VulkanUtil.h"

namespace Ailurus
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL
        DebugReportExtCallback(
            VkDebugReportFlagsEXT,
            VkDebugReportObjectTypeEXT,
            std::uint64_t,
            std::size_t,
            std::int32_t,
            const char*,
            const char* pMessage,
            void*)
    {
        Logger::LogError(pMessage);
        return VK_FALSE;
    }

    Renderer::Renderer(const GetWindowSizeCallback& getWindowSize,
        const GetWindowInstanceExtension& getWindowInstExt,
        const WindowCreateSurfaceCallback& createSurface,
        const WindowDestroySurfaceCallback& destroySurface,
        bool enableValidationLayer)
        : _getWindowSizeCallback(getWindowSize)
        , _getWindowInstExtensionsCallback(getWindowInstExt)
        , _windowDestroySurfaceCallback(destroySurface)
    {
        CreateInstance(enableValidationLayer);

        if (enableValidationLayer)
            CreatDebugReportCallbackExt();

        CreateSurface(createSurface);
    }

    Renderer::~Renderer()
    {
        _windowDestroySurfaceCallback(_vkInstance, _vkSurface);

        if (_vkDebugReportCallbackExt)
        {
            vk::DispatchLoaderDynamic dynamicLoader(_vkInstance, vkGetInstanceProcAddr);
            _vkInstance.destroyDebugReportCallbackEXT(_vkDebugReportCallbackExt, nullptr, dynamicLoader);
        }

        _vkInstance.destroy();
    }

    void Renderer::CreateInstance(bool enableValidation)
    {
        // Validation layers
        static const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";
        std::vector<const char*> validationLayers;
        {
            auto allLayerProperties = vk::enumerateInstanceLayerProperties();

            if (VerboseLog)
            {
                Logger::LogInfo("Supported layer properties:");
                for (auto layerProperty: allLayerProperties)
                    Logger::LogInfo("\t{}", layerProperty.layerName.data());
            }

            if (enableValidation)
            {
                for (auto layerProperty: allLayerProperties)
                {
                    if (layerProperty.layerName == VALIDATION_LAYER_NAME)
                        validationLayers.push_back(VALIDATION_LAYER_NAME);
                }
            }
        }

        // All extensions
        if (VerboseLog)
        {
            auto allExt = vk::enumerateInstanceExtensionProperties();
            Logger::LogInfo("Supported extensions:");
            for (auto ext: allExt)
                Logger::LogInfo("\t{}", ext.extensionName.data());
        }

        // Extensions
        std::vector<const char*> requiredExtensions;
        if (enableValidation)
            requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

        std::vector<const char*> windowInstExtensions = _getWindowInstExtensionsCallback();
        requiredExtensions.insert(requiredExtensions.end(),
            windowInstExtensions.begin(), windowInstExtensions.end());

        // Create instance
        vk::ApplicationInfo applicationInfo;
        applicationInfo.setPApplicationName("Ailurus")
            .setApiVersion(VK_API_VERSION_1_3)
            .setPEngineName("No Engine");

        vk::InstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.setPApplicationInfo(&applicationInfo)
            .setPEnabledLayerNames(validationLayers)
            .setPEnabledExtensionNames(requiredExtensions);

        _vkInstance = vk::createInstance(instanceCreateInfo, nullptr);
    }

    void Renderer::CreatDebugReportCallbackExt()
    {
        auto flags = vk::DebugReportFlagBitsEXT::eWarning
                    | vk::DebugReportFlagBitsEXT::ePerformanceWarning
                    | vk::DebugReportFlagBitsEXT::eError;

        vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;
        debugReportCallbackCreateInfo.setFlags(flags)
            .setPfnCallback(DebugReportExtCallback);

        vk::DispatchLoaderDynamic dynamicLoader(_vkInstance, vkGetInstanceProcAddr);

        _vkDebugReportCallbackExt = _vkInstance.createDebugReportCallbackEXT(debugReportCallbackCreateInfo,
            nullptr, dynamicLoader);
    }

    void Renderer::CreateSurface(const WindowCreateSurfaceCallback& createSurface)
    {
        _vkSurface = createSurface(_vkInstance);
    }

    /*

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

    */
}