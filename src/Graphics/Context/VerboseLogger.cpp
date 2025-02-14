#include "VerboseLogger.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus::VerboseLogger::Vulkan
{
    static bool gEnable = true;

    void SetEnable(bool enable)
    {
        gEnable = enable;
    }

    void LogInstanceLayerProperties()
    {
        if (!gEnable)
            return;

        auto allLayerProperties = vk::enumerateInstanceLayerProperties();

        Logger::LogInfo("Instance layer properties:");
        for (auto layerProperty: allLayerProperties)
            Logger::LogInfo("    {}", layerProperty.layerName.data());
    }

    void LogInstanceExtensionProperties()
    {
        if (!gEnable)
            return;

        auto allExt = vk::enumerateInstanceExtensionProperties();
        Logger::LogInfo("Instance extensions:");
        for (auto ext: allExt)
            Logger::LogInfo("    {}", ext.extensionName.data());
    }

    void LogPhysicalCards(const vk::Instance& vkInstance)
    {
        if (!gEnable)
            return;

        auto graphicCards = vkInstance.enumeratePhysicalDevices();
        Logger::LogInfo("All graphic cards:");
        for (auto& graphicCard: graphicCards)
        {
            auto property = graphicCard.getProperties();
            Logger::LogInfo("    {}", property.deviceName.data());
        }
    }

    void LogChosenPhysicalCard(const vk::PhysicalDevice& vkPhysicalDevice, const vk::SurfaceKHR& vkSurface)
    {
        if (!gEnable)
            return;

        auto physicalDeviceProperty = vkPhysicalDevice.getProperties();
        Logger::LogInfo("Choose physical device: {}, API version: {}, vendor id: {}", physicalDeviceProperty.deviceName.data(),
            physicalDeviceProperty.apiVersion, physicalDeviceProperty.vendorID);

        auto queueFamilyProperties = vkPhysicalDevice.getQueueFamilyProperties();
        Logger::LogInfo("    Queue family size: {}", queueFamilyProperties.size());
        for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
        {
            auto& queueProperty = queueFamilyProperties[i];

            bool canPresent = vkPhysicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), vkSurface);
            bool canGraphic = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eGraphics);
            bool canCompute = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eCompute);
            bool canTransfer = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eTransfer);

            Logger::LogInfo("        {}:\tPresent: {},\tGraphic: {}\t, Compute: {}\t, Transfer: {}",
                i, canPresent, canGraphic, canCompute, canTransfer);
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
