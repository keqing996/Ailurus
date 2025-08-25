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

	auto VulkanHelper::ConvertToVkEnum(MultiSamplingType t) -> vk::SampleCountFlagBits
	{
		switch (t)
		{
			case MultiSamplingType::None:
				return vk::SampleCountFlagBits::e1;
			case MultiSamplingType::MSAAx2:
				return vk::SampleCountFlagBits::e2;
			case MultiSamplingType::MSAAx4:
				return vk::SampleCountFlagBits::e4;
			case MultiSamplingType::MSAAx8:
				return vk::SampleCountFlagBits::e8;
			case MultiSamplingType::MSAAx16:
				return vk::SampleCountFlagBits::e16;
			case MultiSamplingType::MSAAx32:
				return vk::SampleCountFlagBits::e32;
			case MultiSamplingType::MSAAx64:
				return vk::SampleCountFlagBits::e64;
		}

		Logger::LogError("Fail to convert multi sampling type to vk enum, multi sampling type = {}",
			EnumReflection<MultiSamplingType>::ToString(t));
		return vk::SampleCountFlagBits::e1;
	}

	auto VulkanHelper::ConvertToVkEnum(StencilLoadType t) -> vk::AttachmentLoadOp
	{
		switch (t)
		{
			case StencilLoadType::None:
				return vk::AttachmentLoadOp::eDontCare;
			case StencilLoadType::Clear:
				return vk::AttachmentLoadOp::eClear;
			case StencilLoadType::Load:
				return vk::AttachmentLoadOp::eLoad;
		}

		Logger::LogError("Fail to convert stencil load type to vk enum, stencil load type = {}",
			EnumReflection<StencilLoadType>::ToString(t));
		return vk::AttachmentLoadOp::eDontCare;
	}

	auto VulkanHelper::ConvertToVkEnum(StencilWriteType t) -> vk::AttachmentStoreOp
	{
		switch (t)
		{
			case StencilWriteType::None:
				return vk::AttachmentStoreOp::eDontCare;
			case StencilWriteType::Write:
				return vk::AttachmentStoreOp::eStore;
		}

		Logger::LogError("Fail to convert stencil write type to vk enum, stencil write type = {}",
			EnumReflection<StencilWriteType>::ToString(t));
		return vk::AttachmentStoreOp::eDontCare;
	}

} // namespace Ailurus