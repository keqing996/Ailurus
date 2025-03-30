#include <unordered_set>
#include <mutex>
#include <optional>
#include "VulkanContext.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"
#include "Vulkan/SwapChain/SwapChain.h"
#include "Vulkan/Airport/Airport.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Ailurus
{
	namespace Verbose
	{
		static vk::Bool32 VKAPI_PTR DebugCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT messageType,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			Logger::LogError(pCallbackData->pMessage);
			return VK_FALSE;
		}

		static void LogInstanceLayerProperties()
		{
			auto allLayerProperties = vk::enumerateInstanceLayerProperties();

			Logger::LogInfo("Instance layer properties:");
			for (auto layerProperty : allLayerProperties)
				Logger::LogInfo("    {}", layerProperty.layerName.data());
		}

		static void LogInstanceExtensionProperties()
		{
			auto allExt = vk::enumerateInstanceExtensionProperties();
			Logger::LogInfo("Instance extensions:");
			for (auto ext : allExt)
				Logger::LogInfo("    {}", ext.extensionName.data());
		}

		static void LogPhysicalCards(vk::Instance vkInstance)
		{
			auto graphicCards = vkInstance.enumeratePhysicalDevices();
			Logger::LogInfo("All graphic cards:");
			for (auto& graphicCard : graphicCards)
			{
				auto property = graphicCard.getProperties();
				Logger::LogInfo("    {}", property.deviceName.data());
			}
		}

		static void LogChosenPhysicalCard(const vk::PhysicalDevice& vkPhysicalDevice, vk::SurfaceKHR vkSurface)
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
	} // namespace Verbose

	bool VulkanContext::enableValidation = true;
	bool VulkanContext::_initialized = false;
	vk::Instance VulkanContext::_vkInstance = nullptr;
	vk::DebugUtilsMessengerEXT VulkanContext::_vkDebugUtilsMessenger = nullptr;
	vk::PhysicalDevice VulkanContext::_vkPhysicalDevice = nullptr;
	vk::SurfaceKHR VulkanContext::_vkSurface = nullptr;
	vk::Device VulkanContext::_vkDevice = nullptr;
	uint32_t VulkanContext::_presentQueueIndex = 0;
	uint32_t VulkanContext::_graphicQueueIndex = 0;
	uint32_t VulkanContext::_computeQueueIndex = 0;
	vk::Queue VulkanContext::_vkPresentQueue = nullptr;
	vk::Queue VulkanContext::_vkGraphicQueue = nullptr;
	vk::Queue VulkanContext::_vkComputeQueue = nullptr;
	vk::CommandPool VulkanContext::_vkGraphicCommandPool = nullptr;

	std::unique_ptr<SwapChain> VulkanContext::_pSwapChain = nullptr;
	std::unique_ptr<Airport> VulkanContext::_pAirport = nullptr;

	bool VulkanContext::Init(const GetWindowInstanceExtension& getWindowRequiredExtension, const WindowCreateSurfaceCallback& createSurface)
	{
		if (_initialized)
			return true;

		PrepareDispatcher();

		Verbose::LogInstanceLayerProperties();
		Verbose::LogInstanceExtensionProperties();

		CreateInstance(getWindowRequiredExtension);
		CreatDebugUtilsMessenger();
		CreateSurface(createSurface);

		Verbose::LogPhysicalCards(_vkInstance);

		ChoosePhysicsDevice();

		Verbose::LogChosenPhysicalCard(_vkPhysicalDevice, _vkSurface);

		if (!CreateLogicalDevice())
			return false;

		CreateCommandPool();

		_initialized = true;

		CreateDynamicContext();

		return true;
	}

	void VulkanContext::Destroy(const WindowDestroySurfaceCallback& destroySurface)
	{
		if (!_initialized)
			return;

		DestroyDynamicContext();

		if (_vkDevice)
		{
			_vkDevice.destroyCommandPool(_vkGraphicCommandPool);
			_vkDevice.destroy();
		}

		if (_vkSurface)
			destroySurface(_vkInstance, _vkSurface);

		if (_vkInstance)
			_vkInstance.destroy();

		_initialized = false;
	}

	vk::Device VulkanContext::GetDevice()
	{
		return _vkDevice;
	}

	vk::PhysicalDevice VulkanContext::GetPhysicalDevice()
	{
		return _vkPhysicalDevice;
	}

	vk::SurfaceKHR VulkanContext::GetSurface()
	{
		return _vkSurface;
	}

	uint32_t VulkanContext::GetPresentQueueIndex()
	{
		return _presentQueueIndex;
	}

	uint32_t VulkanContext::GetGraphicQueueIndex()
	{
		return _graphicQueueIndex;
	}

	vk::Queue VulkanContext::GetPresentQueue()
	{
		return _vkPresentQueue;
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

	vk::CommandPool VulkanContext::GetCommandPool()
	{
		return _vkGraphicCommandPool;
	}

	void VulkanContext::RebuildDynamicContext()
	{
		if (!_initialized)
			return;

		DestroyDynamicContext();
		CreateDynamicContext();
	}

	SwapChain* VulkanContext::GetSwapChain()
	{
		return _pSwapChain.get();
	}

	Airport* VulkanContext::GetAirport()
	{
		return _pAirport.get();
	}

	void VulkanContext::PrepareDispatcher()
	{
		vk::detail::DynamicLoader loader;
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
			loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
	}

	void VulkanContext::CreateInstance(const GetWindowInstanceExtension& getWindowRequiredExtension)
	{
		// Validation layers
		static const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";
		std::vector<const char*> validationLayers;
		if (enableValidation)
		{
			auto allLayerProperties = vk::enumerateInstanceLayerProperties();
			for (auto layerProperty : allLayerProperties)
			{
				if (std::string(layerProperty.layerName.data()) == VALIDATION_LAYER_NAME)
					validationLayers.push_back(VALIDATION_LAYER_NAME);
			}
		}

		// Extensions
		std::vector<const char*> extensions = getWindowRequiredExtension();
		if (enableValidation)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#if AILURUS_PLATFORM_MAC
		extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

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

#if AILURUS_PLATFORM_MAC
		instanceCreateInfo.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif

		_vkInstance = vk::createInstance(instanceCreateInfo, nullptr);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(_vkInstance);
	}

	void VulkanContext::CreatDebugUtilsMessenger()
	{
		if (!enableValidation)
			return;

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

	void VulkanContext::CreateSurface(const WindowCreateSurfaceCallback& createSurface)
	{
		_vkSurface = createSurface(_vkInstance);
	}

	void VulkanContext::ChoosePhysicsDevice()
	{
		auto graphicCards = _vkInstance.enumeratePhysicalDevices();

		vk::PhysicalDeviceType lastFoundDeviceType = vk::PhysicalDeviceType::eOther;
		for (auto& graphicCard : graphicCards)
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
		std::optional<uint32_t> optPresentQueue = std::nullopt;
		std::optional<uint32_t> optGraphicQueue = std::nullopt;
		std::optional<uint32_t> optComputeQueue = std::nullopt;
		auto queueFamilyProperties = _vkPhysicalDevice.getQueueFamilyProperties();
		for (std::size_t i = 0; i < queueFamilyProperties.size(); ++i)
		{
			if (_vkPhysicalDevice.getSurfaceSupportKHR(i, _vkSurface) && !optPresentQueue.has_value())
				optPresentQueue = static_cast<uint32_t>(i);

			auto& property = queueFamilyProperties[i];
			if ((property.queueFlags & vk::QueueFlagBits::eGraphics) && !optGraphicQueue.has_value())
				optGraphicQueue = static_cast<uint32_t>(i);

			if ((property.queueFlags & vk::QueueFlagBits::eCompute) && !optComputeQueue.has_value())
				optComputeQueue = static_cast<uint32_t>(i);

			if (optPresentQueue.has_value() && optGraphicQueue.has_value() && optComputeQueue.has_value())
				break;
		}

		if (!optPresentQueue.has_value() || !optGraphicQueue.has_value() || !optComputeQueue.has_value())
		{
			Logger::LogError("Fail to get queues.");
			return false;
		}

		_presentQueueIndex = *optPresentQueue;
		_graphicQueueIndex = *optGraphicQueue;
		_computeQueueIndex = *optComputeQueue;

		// Queue create info
		const float queuePriority = 1.0f;
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfoList;
		queueCreateInfoList.emplace_back();
		queueCreateInfoList.back().setQueueCount(1).setQueuePriorities(queuePriority).setQueueFamilyIndex(_presentQueueIndex);

		if (_graphicQueueIndex != _presentQueueIndex)
		{
			queueCreateInfoList.emplace_back();
			queueCreateInfoList.back()
				.setQueueCount(1)
				.setQueuePriorities(queuePriority)
				.setQueueFamilyIndex(_graphicQueueIndex);
		}

		if (_computeQueueIndex != _presentQueueIndex)
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
		std::array extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if AILURUS_PLATFORM_MAC
			//VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
			// VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
			"VK_KHR_portability_subset"
#endif
		};

		vk::DeviceCreateInfo deviceCreateInfo;
		deviceCreateInfo
			.setPEnabledExtensionNames(extensions)
			.setQueueCreateInfos(queueCreateInfoList)
			.setPEnabledFeatures(&physicalDeviceFeatures);

		_vkDevice = _vkPhysicalDevice.createDevice(deviceCreateInfo);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(_vkDevice);

		_vkPresentQueue = _vkDevice.getQueue(_presentQueueIndex, 0);
		_vkGraphicQueue = _vkDevice.getQueue(_graphicQueueIndex, 0);
		_vkComputeQueue = _vkDevice.getQueue(_computeQueueIndex, 0);

		return true;
	}

	void VulkanContext::CreateCommandPool()
	{
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(GetGraphicQueueIndex());

		_vkGraphicCommandPool = _vkDevice.createCommandPool(poolInfo);
	}

	void VulkanContext::CreateDynamicContext()
	{
		Vector2i windowSize = Application::GetSize();
		_pSwapChain = std::make_unique<SwapChain>(windowSize.x(), windowSize.y());
		_pAirport = std::make_unique<Airport>();
	}

	void VulkanContext::DestroyDynamicContext()
	{
		_pAirport.reset();
		_pSwapChain.reset();
	}
} // namespace Ailurus
