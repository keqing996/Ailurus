#include "Ailurus/Graphics/Context/VulkanContext.h"
#include "VerboseLogger.h"
#include "Ailurus/Utility/Logger.h"

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

    VulkanContext::VulkanContext(const GetWindowSizeCallback& getWindowSize,
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

    VulkanContext::~VulkanContext()
    {
        _vkLogicalDevice.destroy();

        _windowDestroySurfaceCallback(_vkInstance, _vkSurface);

        if (_vkDebugReportCallbackExt)
        {
            vk::DispatchLoaderDynamic dynamicLoader(_vkInstance, vkGetInstanceProcAddr);
            _vkInstance.destroyDebugReportCallbackEXT(_vkDebugReportCallbackExt, nullptr, dynamicLoader);
        }

        _vkInstance.destroy();
    }

    vk::Instance VulkanContext::GetInstance() const
    {
        return _vkInstance;
    }

    vk::SurfaceKHR VulkanContext::GetSurface() const
    {
        return _vkSurface;
    }

    vk::PhysicalDevice VulkanContext::GetPhysicalDevice() const
    {
        return _vkPhysicalDevice;
    }

    vk::Device VulkanContext::GetLogicalDevice() const
    {
        return _vkLogicalDevice;
    }

    const QueueIndex& VulkanContext::GetQueueIndex() const
    {
        return _queueIndex;
    }

    vk::Queue VulkanContext::GetGraphicQueue() const
    {
        return _vkGraphicQueue;
    }

    vk::Queue VulkanContext::GetPresentQueue() const
    {
        return _vkPresentQueue;
    }

    void VulkanContext::CreateInstance(bool enableValidation)
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
        applicationInfo
            .setPApplicationName("Ailurus")
            .setApiVersion(VK_API_VERSION_1_3)
            .setPEngineName("No Engine");

        vk::InstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo
            .setPApplicationInfo(&applicationInfo)
            .setPEnabledLayerNames(validationLayers)
            .setPEnabledExtensionNames(requiredExtensions);

        _vkInstance = vk::createInstance(instanceCreateInfo, nullptr);
    }

    void VulkanContext::CreatDebugReportCallbackExt()
    {
        auto flags = vk::DebugReportFlagBitsEXT::eWarning
                    | vk::DebugReportFlagBitsEXT::ePerformanceWarning
                    | vk::DebugReportFlagBitsEXT::eError;

        vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;
        debugReportCallbackCreateInfo
            .setFlags(flags)
            .setPfnCallback(DebugReportExtCallback);

        vk::DispatchLoaderDynamic dynamicLoader(_vkInstance, vkGetInstanceProcAddr);

        _vkDebugReportCallbackExt = _vkInstance.createDebugReportCallbackEXT(debugReportCallbackCreateInfo,
            nullptr, dynamicLoader);
    }

    void VulkanContext::CreateSurface(const WindowCreateSurfaceCallback& createSurface)
    {
        _vkSurface = createSurface(_vkInstance);
    }

    void VulkanContext::ChoosePhysicsDevice()
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

    void VulkanContext::CreateLogicDevice()
    {
        // Find graphic queue and present queue.
        auto queueFamilyProperties = _vkPhysicalDevice.getQueueFamilyProperties();
        for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
        {
            auto& property = queueFamilyProperties[i];
            if ((property.queueFlags & vk::QueueFlagBits::eGraphics) && !_queueIndex.graphicQueueIndex.has_value())
                _queueIndex.graphicQueueIndex = static_cast<uint32_t>(i);

            bool canPresent = _vkPhysicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), _vkSurface);
            if (canPresent && !_queueIndex.presentQueueIndex.has_value())
                _queueIndex.presentQueueIndex = static_cast<uint32_t>(i);

            if (_queueIndex.graphicQueueIndex.has_value()&& _queueIndex.presentQueueIndex.has_value())
                break;
        }

        // Queue create info
        const float queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfoList;
        queueCreateInfoList.emplace_back();
        queueCreateInfoList.back().setQueueCount(1)
            .setQueuePriorities(queuePriority)
            .setQueueFamilyIndex(_queueIndex.graphicQueueIndex.value());

        if (_queueIndex.presentQueueIndex.value() != _queueIndex.graphicQueueIndex.value())
        {
            queueCreateInfoList.emplace_back();
            queueCreateInfoList.back()
                .setQueueCount(1)
                .setQueuePriorities(queuePriority)
                .setQueueFamilyIndex(_queueIndex.presentQueueIndex.value());
        }

        // Features
        vk::PhysicalDeviceFeatures physicalDeviceFeatures;
        physicalDeviceFeatures.setSamplerAnisotropy(true);

        // Swap chain is required
        const char* extensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        vk::DeviceCreateInfo deviceCreateInfo;
        deviceCreateInfo
            .setPEnabledExtensionNames(extensions)
            .setQueueCreateInfos(queueCreateInfoList)
            .setPEnabledFeatures(&physicalDeviceFeatures);

        _vkLogicalDevice = _vkPhysicalDevice.createDevice(deviceCreateInfo);
        _vkGraphicQueue = _vkLogicalDevice.getQueue(_queueIndex.graphicQueueIndex.value(), 0);
        _vkPresentQueue = _vkLogicalDevice.getQueue(_queueIndex.presentQueueIndex.value(), 0);
    }
}