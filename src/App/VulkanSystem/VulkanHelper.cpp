#include "VulkanHelper.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{

	vk::Bool32 VKAPI_PTR VulkanHelper::DebugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		vk::DebugUtilsMessageTypeFlagsEXT messageType,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		Logger::LogError(pCallbackData->pMessage);
		return VK_FALSE;
	}

	void VulkanHelper::LogInstanceLayerProperties()
	{
		auto allLayerProperties = vk::enumerateInstanceLayerProperties();

		Logger::LogInfo("Instance layer properties:");
		for (auto layerProperty : allLayerProperties)
			Logger::LogInfo("    {}", layerProperty.layerName.data());
	}

	void VulkanHelper::LogInstanceExtensionProperties()
	{
		auto allExt = vk::enumerateInstanceExtensionProperties();
		Logger::LogInfo("Instance extensions:");
		for (auto ext : allExt)
			Logger::LogInfo("    {}", ext.extensionName.data());
	}

	void VulkanHelper::LogPhysicalCards(vk::Instance vkInstance)
	{
		auto graphicCards = vkInstance.enumeratePhysicalDevices();
		Logger::LogInfo("All graphic cards:");
		for (auto& graphicCard : graphicCards)
		{
			auto property = graphicCard.getProperties();
			Logger::LogInfo("    {}", property.deviceName.data());
		}
	}

	void VulkanHelper::LogChosenPhysicalCard(const vk::PhysicalDevice& vkPhysicalDevice, vk::SurfaceKHR vkSurface)
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
			bool canPresent = vkPhysicalDevice.getSurfaceSupportKHR(i, vkSurface);
			bool canGraphic = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eGraphics);
			bool canCompute = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eCompute);
			bool canTransfer = static_cast<bool>(queueProperty.queueFlags & vk::QueueFlagBits::eTransfer);

			Logger::LogInfo("        {}:\tPresent: {}\t, Graphic: {}\t, Compute: {}\t, Transfer: {}",
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
} // namespace Ailurus