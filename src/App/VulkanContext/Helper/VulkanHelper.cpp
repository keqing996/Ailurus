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
		// Log based on severity
		const char* msg = pCallbackData && pCallbackData->pMessage ? pCallbackData->pMessage : "(null)";
		if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
			Logger::LogError("Vulkan Debug: {}", msg);
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
			Logger::LogWarn("Vulkan Warning: {}", msg);
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
			Logger::LogInfo("Vulkan Info: {}", msg);
		else
			Logger::LogInfo("Vulkan: {}", msg);

		return VK_FALSE;
	}

	void VulkanHelper::LogInstanceLayerProperties()
	{
		try
		{
			auto allLayerProperties = vk::enumerateInstanceLayerProperties();

			Logger::LogInfo("Instance layer properties:");
			for (auto layerProperty : allLayerProperties)
				Logger::LogInfo("    {}", layerProperty.layerName.data());
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to enumerate instance layer properties: {}", e.what());
		}
	}

	void VulkanHelper::LogInstanceExtensionProperties()
	{
		try
		{
			auto allExt = vk::enumerateInstanceExtensionProperties();
			Logger::LogInfo("Instance extensions:");
			for (auto ext : allExt)
				Logger::LogInfo("    {}", ext.extensionName.data());
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to enumerate instance extension properties: {}", e.what());
		}
	}

	void VulkanHelper::LogPhysicalCards(vk::Instance vkInstance)
	{
		try
		{
			auto graphicCards = vkInstance.enumeratePhysicalDevices();
			Logger::LogInfo("All graphic cards:");
			for (auto& graphicCard : graphicCards)
			{
				auto property = graphicCard.getProperties();
				Logger::LogInfo("    {}", property.deviceName.data());
			}
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to enumerate physical devices: {}", e.what());
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

	vk::ShaderStageFlagBits VulkanHelper::GetShaderStage(ShaderStage stage)
	{
		vk::ShaderStageFlagBits vkStage = vk::ShaderStageFlagBits::eAll;
		switch (stage)
		{
			case ShaderStage::Vertex:
				vkStage = vk::ShaderStageFlagBits::eVertex;
				break;
			case ShaderStage::Fragment:
				vkStage = vk::ShaderStageFlagBits::eFragment;
				break;
		}

		return vkStage;
	}

	uint32_t VulkanHelper::SizeOf(AttributeType type)
	{
		switch (type)
		{
			case AttributeType::Position:
				return sizeof(float) * 3;
			case AttributeType::Normal:
				return sizeof(float) * 3;
			case AttributeType::TexCoord:
				return sizeof(float) * 2;
			case AttributeType::Tangent:
				return sizeof(float) * 3;
			case AttributeType::Bitangent:
				return sizeof(float) * 3;
			case AttributeType::Color:
				return sizeof(float) * 4;
		}

		Logger::LogError("Fail to get attribute size, attribute type = {}",
			EnumReflection<AttributeType>::ToString(type));
		return 0;
	}

	vk::Format VulkanHelper::GetFormat(AttributeType type)
	{
		const auto bytes = SizeOf(type);
		switch (bytes)
		{
			case sizeof(float) * 2:
				return vk::Format::eR32G32Sfloat;
			case sizeof(float) * 3:
				return vk::Format::eR32G32B32Sfloat;
			case sizeof(float) * 4:
				return vk::Format::eR32G32B32A32Sfloat;
			default:
				Logger::LogError("Fail to convert attribute type to vk format, attribute type = {}",
					EnumReflection<AttributeType>::ToString(type));
				return vk::Format::eUndefined;
		}
	}

	uint32_t VulkanHelper::CalculateVertexLayoutStride(const std::vector<AttributeType>& attributes)
	{
		uint32_t stride = 0;
		for (auto i = 0; i < attributes.size(); i++)
			stride += SizeOf(attributes[i]);
		return stride;
	}

	uint32_t VulkanHelper::SizeOf(IndexBufferFormat type)
	{
		switch (type)
		{
			case IndexBufferFormat::UInt16:
				return sizeof(uint16_t);
			case IndexBufferFormat::UInt32:
				return sizeof(uint32_t);
		}

		Logger::LogError("Fail to get index format size, index format type = {}",
			EnumReflection<IndexBufferFormat>::ToString(type));
		return 0;
	}

} // namespace Ailurus