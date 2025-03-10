#include <unordered_set>
#include <mutex>
#include "VulkanContext.h"
#include "Ailurus/Utility/Logger.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Ailurus
{
    namespace Verbose
    {
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData)
        {
            Logger::LogError(pCallbackData->pMessage);
            return VK_FALSE;
        }

        static void LogInstanceLayerProperties()
        {
            auto allLayerProperties = vk::enumerateInstanceLayerProperties();

            Logger::LogInfo("Instance layer properties:");
            for (auto layerProperty: allLayerProperties)
                Logger::LogInfo("    {}", layerProperty.layerName.data());
        }

        static void LogInstanceExtensionProperties()
        {
            auto allExt = vk::enumerateInstanceExtensionProperties();
            Logger::LogInfo("Instance extensions:");
            for (auto ext: allExt)
                Logger::LogInfo("    {}", ext.extensionName.data());
        }

        static void LogPhysicalCards(const vk::Instance& vkInstance)
        {
            auto graphicCards = vkInstance.enumeratePhysicalDevices();
            Logger::LogInfo("All graphic cards:");
            for (auto& graphicCard: graphicCards)
            {
                auto property = graphicCard.getProperties();
                Logger::LogInfo("    {}", property.deviceName.data());
            }
        }

        static void LogChosenPhysicalCard(const vk::PhysicalDevice& vkPhysicalDevice)
        {
            auto physicalDeviceProperty = vkPhysicalDevice.getProperties();
            Logger::LogInfo("Choose physical device: {}, API version: {}, vendor id: {}",
                            physicalDeviceProperty.deviceName.data(),
                            physicalDeviceProperty.apiVersion, physicalDeviceProperty.vendorID);

            auto queueFamilyProperties = vkPhysicalDevice.getQueueFamilyProperties();
            Logger::LogInfo("    Queue family size: {}", queueFamilyProperties.size());
            for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
            {
                auto& queueProperty = queueFamilyProperties[i];
                bool canGraphic = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eGraphics);
                bool canCompute = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eCompute);
                bool canTransfer = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eTransfer);

                Logger::LogInfo("        {}:\tGraphic: {}\t, Compute: {}\t, Transfer: {}",
                                i, canGraphic, canCompute, canTransfer);
            }

#if false // Too long to show details
            Logger::LogInfo("    Physical device layer properties:");
            auto allLayerProperties = vkPhysicalDevice.enumerateDeviceLayerProperties();
            for (auto layerProperty: allLayerProperties)
                Logger::LogInfo("        {}", layerProperty.layerName.data());

            Logger::LogInfo("    Physical device extensions:");
            auto allExt = vkPhysicalDevice.enumerateDeviceExtensionProperties();
            for (auto ext: allExt)
                Logger::LogInfo("        {}", ext.extensionName.data());
#endif
        }
    }

    VulkanContext::GetWindowInstanceExtension   VulkanContext::_getWindowRequiredExtension  = nullptr;
    VulkanContext::WindowCreateSurfaceCallback  VulkanContext::_createSurface               = nullptr;
    VulkanContext::WindowDestroySurfaceCallback VulkanContext::_destroySurface              = nullptr;

    bool                        VulkanContext::enableValidation         = true;
    vk::Instance                VulkanContext::_vkInstance              = nullptr;
    vk::DebugUtilsMessengerEXT  VulkanContext::_vkDebugUtilsMessenger   = nullptr;
    vk::PhysicalDevice          VulkanContext::_vkPhysicalDevice        = nullptr;
    vk::SurfaceKHR              VulkanContext::_vkSurface               = nullptr;
    vk::Device                  VulkanContext::_vkDevice                = nullptr;
    uint32_t                    VulkanContext::_graphicQueueIndex       = 0;
    uint32_t                    VulkanContext::_computeQueueIndex       = 0;
    vk::Queue                   VulkanContext::_vkGraphicQueue          = nullptr;
    vk::Queue                   VulkanContext::_vkComputeQueue          = nullptr;

    void VulkanContext::SetCallbackGetWindowInstanceExtension(const GetWindowInstanceExtension& f)
    {
        _getWindowRequiredExtension = f;
    }

    void VulkanContext::SetCallbackWindowCreateSurfaceCallback(const WindowCreateSurfaceCallback& f)
    {
        _createSurface = f;
    }

    void VulkanContext::SetCallbackWindowDestroySurfaceCallback(const WindowDestroySurfaceCallback& f)
    {
        _destroySurface = f;
    }

    void VulkanContext::Init()
    {

    }

    void VulkanContext::Destroy()
    {

    }

    bool VulkanContext::Register(const Renderer* pRenderer, vk::SurfaceKHR* outSurface)
    {
        std::lock_guard lockGuard(gRegisterMutex);
        {
            if (gRegisteredRenders.contains(pRenderer))
                return true;

            // Init vulkan if empty
            if (gRegisteredRenders.empty() && !InitVulkan())
            {
                DestroyVulkan();
                return false;
            }

            // Check surface
            vk::SurfaceKHR surface = _createSurface(_vkInstance);
            bool canPresent = _vkPhysicalDevice.getSurfaceSupportKHR(_graphicQueueIndex, surface);
            if (!canPresent)
            {
                Logger::LogError("Graphic queue can not present target surface.");

                if (gRegisteredRenders.empty())
                    DestroyVulkan();

                return false;
            }

            *outSurface = surface;
            gRegisteredRenders.insert(pRenderer);

            return true;
        }
    }

    void VulkanContext::Unregister(const Renderer* pRenderer)
    {
        std::lock_guard lockGuard(gRegisterMutex);
        {
            if (gRegisteredRenders.contains(pRenderer))
            {
                _destroySurface(_vkInstance, pRenderer->GetSurface());
                gRegisteredRenders.erase(pRenderer);
            }

            if (gRegisteredRenders.empty())
                DestroyVulkan();
        }
    }

    vk::Device VulkanContext::GetDevice()
    {
        return _vkDevice;
    }

    uint32_t VulkanContext::GetGraphicQueueIndex()
    {
        return _graphicQueueIndex;
    }

    vk::Queue VulkanContext::GetGraphicQueue()
    {
        return _vkGraphicQueue;
    }

    uint32_t VulkanContext::GetComputeQueueIndex()
    {
        return _computeQueueIndex;
    }

    vk::Queue VulkanContext::GetComputeQueue()
    {
        return _vkComputeQueue;
    }

    bool VulkanContext::InitVulkan()
    {
        vk::detail::DynamicLoader loader;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
            loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        Verbose::LogInstanceLayerProperties();
        Verbose::LogInstanceExtensionProperties();

        CreateInstance();

        VULKAN_HPP_DEFAULT_DISPATCHER.init(_vkInstance);

        if (enableValidation)
            CreatDebugUtilsMessenger();

        Verbose::LogPhysicalCards(_vkInstance);

        ChoosePhysicsDevice();

        Verbose::LogChosenPhysicalCard(_vkPhysicalDevice);

        if (!CreateLogicalDevice())
            return false;

        VULKAN_HPP_DEFAULT_DISPATCHER.init(_vkDevice);

        return true;
    }

    void VulkanContext::DestroyVulkan()
    {
        if (_vkDevice)
            _vkDevice.destroy();

        if (_vkInstance)
            _vkInstance.destroy();
    }

    void VulkanContext::CreateInstance()
    {
        // Validation layers
        static const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";
        std::vector<const char*> validationLayers;
        if (enableValidation)
        {
            auto allLayerProperties = vk::enumerateInstanceLayerProperties();
            for (auto layerProperty: allLayerProperties)
            {
                if (std::string(layerProperty.layerName.data()) == VALIDATION_LAYER_NAME)
                    validationLayers.push_back(VALIDATION_LAYER_NAME);
            }
        }

        // Extensions
        std::vector<const char*> extensions = _getWindowRequiredExtension();
        if (enableValidation)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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
                .setPEnabledExtensionNames(extensions);

        _vkInstance = vk::createInstance(instanceCreateInfo, nullptr);
    }

    void VulkanContext::CreatDebugUtilsMessenger()
    {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo;
        createInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                        | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                        | vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
            .setPUserData(nullptr)
            .setPfnUserCallback(Verbose::DebugCallback);

        _vkDebugUtilsMessenger = _vkInstance.createDebugUtilsMessengerEXT(createInfo);
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

    bool VulkanContext::CreateLogicalDevice()
    {
        // Find graphic queue and present queue.
        std::optional<uint32_t> optGraphicQueue = std::nullopt;
        std::optional<uint32_t> optComputeQueue = std::nullopt;
        auto queueFamilyProperties = _vkPhysicalDevice.getQueueFamilyProperties();
        for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
        {
            auto& property = queueFamilyProperties[i];
            if ((property.queueFlags & vk::QueueFlagBits::eGraphics) && !optGraphicQueue.has_value())
                optGraphicQueue = static_cast<uint32_t>(i);

            if ((property.queueFlags & vk::QueueFlagBits::eCompute) && !optComputeQueue.has_value())
                optComputeQueue = static_cast<uint32_t>(i);

            if (optGraphicQueue.has_value() && optComputeQueue.has_value())
                break;
        }

        if (!optGraphicQueue.has_value() || !optComputeQueue.has_value())
        {
            Logger::LogError("Fail to get graphic queue and compute queue.");
            return false;
        }

        _graphicQueueIndex = *optGraphicQueue;
        _computeQueueIndex = *optComputeQueue;

        // Queue create info
        const float queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfoList;
        queueCreateInfoList.emplace_back();
        queueCreateInfoList.back().setQueueCount(1)
                .setQueuePriorities(queuePriority)
                .setQueueFamilyIndex(_graphicQueueIndex);

        if (_computeQueueIndex != _graphicQueueIndex)
        {
            queueCreateInfoList.emplace_back();
            queueCreateInfoList.back()
                    .setQueueCount(1)
                    .setQueuePriorities(queuePriority)
                    .setQueueFamilyIndex(_computeQueueIndex);
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

        _vkDevice = _vkPhysicalDevice.createDevice(deviceCreateInfo);
        _vkGraphicQueue = _vkDevice.getQueue(_graphicQueueIndex, 0);
        _vkComputeQueue = _vkDevice.getQueue(_computeQueueIndex, 0);

        return true;
    }

}
