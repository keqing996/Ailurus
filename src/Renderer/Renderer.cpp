#include <algorithm>
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Renderer/Renderer.h"
#include "VerboseLogger/VerboseLogger.h"

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
        VerboseLogger::Vulkan::LogInstanceLayerProperties();
        VerboseLogger::Vulkan::LogInstanceExtensionProperties();

        CreateInstance(enableValidationLayer);

        if (enableValidationLayer)
            CreatDebugReportCallbackExt();

        CreateSurface(createSurface);

        VerboseLogger::Vulkan::LogPhysicalCards(_vkInstance);

        ChoosePhysicsDevice();

        VerboseLogger::Vulkan::LogChosenPhysicalCard(_vkPhysicalDevice, _vkSurface);

        CreateLogicDevice();
    }

    Renderer::~Renderer()
    {
        _vkLogicDevice.destroy();

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
        if (enableValidation)
        {
            auto allLayerProperties = vk::enumerateInstanceLayerProperties();
            for (auto layerProperty: allLayerProperties)
            {
                if (layerProperty.layerName == VALIDATION_LAYER_NAME)
                    validationLayers.push_back(VALIDATION_LAYER_NAME);
            }
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

    void Renderer::ChoosePhysicsDevice()
    {
        auto graphicCards = _vkInstance.enumeratePhysicalDevices();

        vk::PhysicalDeviceType lastFoundDeviceType = vk::PhysicalDeviceType::eOther;
        for (auto& graphicCard: graphicCards)
        {
            auto property = graphicCard.getProperties();

            // Discrete gpu found, no need to continue.
            if (property.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            {
                _vkPhysicalDevice = graphicCard;
                break;
            }

            // Integrated gpu found, record it and keep finding discrete gpu.
            if (property.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
            {
                _vkPhysicalDevice = graphicCard;
                lastFoundDeviceType = vk::PhysicalDeviceType::eIntegratedGpu;
            }

            // Cpu found, record it only if that no gpu was found.
            if (property.deviceType == vk::PhysicalDeviceType::eCpu
                && lastFoundDeviceType != vk::PhysicalDeviceType::eIntegratedGpu)
            {
                _vkPhysicalDevice = graphicCard;
                lastFoundDeviceType = vk::PhysicalDeviceType::eCpu;
            }
        }
    }

    void Renderer::CreateLogicDevice()
    {
        struct QueueIndex
        {
            std::optional<uint32_t> graphicQueueIndex = std::nullopt;
            std::optional<uint32_t> presentQueueIndex = std::nullopt;
        };

        QueueIndex queueIndex;

        // Find graphic queue and present queue.
        auto queueFamilyProperties = _vkPhysicalDevice.getQueueFamilyProperties();
        for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
        {
            auto& property = queueFamilyProperties[i];
            if ((property.queueFlags & vk::QueueFlagBits::eGraphics) && !queueIndex.graphicQueueIndex.has_value())
                queueIndex.graphicQueueIndex = static_cast<uint32_t>(i);

            bool canPresent = _vkPhysicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), _vkSurface);
            if (canPresent && !queueIndex.presentQueueIndex.has_value())
                queueIndex.presentQueueIndex = static_cast<uint32_t>(i);

            if (queueIndex.graphicQueueIndex.has_value()&& queueIndex.presentQueueIndex.has_value())
                break;
        }

        // Queue create info
        const float queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfoList;
        queueCreateInfoList.emplace_back();
        queueCreateInfoList.back().setQueueCount(1)
            .setQueuePriorities(queuePriority)
            .setQueueFamilyIndex(queueIndex.graphicQueueIndex.value());

        if (queueIndex.presentQueueIndex.value() != queueIndex.graphicQueueIndex.value())
        {
            queueCreateInfoList.emplace_back();
            queueCreateInfoList.back().setQueueCount(1)
                .setQueuePriorities(queuePriority)
                .setQueueFamilyIndex(queueIndex.presentQueueIndex.value());
        }

        // Features
        vk::PhysicalDeviceFeatures physicalDeviceFeatures;
        physicalDeviceFeatures.setSamplerAnisotropy(true);

        // Swap chain is required
        const char* extensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        vk::DeviceCreateInfo deviceCreateInfo;
        deviceCreateInfo.setPEnabledExtensionNames(extensions)
            .setQueueCreateInfos(queueCreateInfoList)
            .setPEnabledFeatures(&physicalDeviceFeatures);

        _vkLogicDevice = _vkPhysicalDevice.createDevice(deviceCreateInfo);
        _vkGraphicQueue = _vkLogicDevice.getQueue(queueIndex.graphicQueueIndex.value(), 0);
        _vkPresentQueue = _vkLogicDevice.getQueue(queueIndex.presentQueueIndex.value(), 0);
    }

    /*

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