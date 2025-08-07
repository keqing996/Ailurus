#include <memory>
#include <optional>
#include <array>
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"
#include "VulkanContext.h"
#include "SwapChain/VulkanSwapChain.h"
#include "Helper/VulkanHelper.h"
#include "Resource/VulkanResourceManager.h"
#include "Vertex/VulkanVertexLayoutManager.h"
#include "Pipeline/VulkanPipelineManager.h"
#include "Flight/VulkanFlightManager.h"
#include "FrameBuffer/VulkanFrameBufferManager.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Ailurus
{
	void VulkanContext::Initialize(const GetWindowInstanceExtension& getWindowRequiredExtension,
		const WindowCreateSurfaceCallback& createSurface, bool enableValidation)
	{
		PrepareDispatcher();

		VulkanHelper::LogInstanceLayerProperties();
		VulkanHelper::LogInstanceExtensionProperties();

		CreateInstance(getWindowRequiredExtension, enableValidation);

		if (enableValidation)
			CreatDebugUtilsMessenger();

		CreateSurface(createSurface);

		VulkanHelper::LogPhysicalCards(_vkInstance);

		ChoosePhysicsDevice();

		VulkanHelper::LogChosenPhysicalCard(_vkPhysicalDevice, _vkSurface);

		if (!CreateLogicalDevice())
			return;

		CreateCommandPool();

		// Create swap chain
		_pSwapChain = std::make_unique<VulkanSwapChain>();

		// Create managers
		_resourceManager = std::make_unique<VulkanResourceManager>();
		_vertexLayoutManager = std::make_unique<VulkanVertexLayoutManager>();
		_pipelineManager = std::make_unique<VulkanPipelineManager>();
		_flightManager = std::make_unique<VulkanFlightManager>(2);

		_initialized = true;
	}

	bool VulkanContext::Initialized()
	{
		return _initialized;
	}

	void VulkanContext::Destroy(const WindowDestroySurfaceCallback& destroySurface)
	{
		if (!_initialized)
			return;

		// Destroy managers
		_flightManager.reset();
		_pipelineManager.reset();
		_vertexLayoutManager.reset();
		_resourceManager.reset();

		// Destroy the swap chain
		if (_pSwapChain)
			_pSwapChain = nullptr;

		// Clear vulkan objects
		if (_vkDevice)
		{
			_vkDevice.destroyCommandPool(_vkGraphicCommandPool);
			_vkDevice.destroy();
			_vkDevice = nullptr;
		}

		if (_vkSurface)
		{
			destroySurface(_vkInstance, _vkSurface);
			_vkSurface = nullptr;
		}

		if (_vkDebugUtilsMessenger)
		{
			_vkInstance.destroyDebugUtilsMessengerEXT(_vkDebugUtilsMessenger);
			_vkDebugUtilsMessenger = nullptr;
		}

		if (_vkInstance)
		{
			_vkInstance.destroy();
			_vkInstance = nullptr;
		}

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

	VulkanResourceManager* VulkanContext::GetResourceManager()
	{
		return _resourceManager.get();
	}

	VulkanVertexLayoutManager* VulkanContext::GetVertexLayoutManager()
	{
		return _vertexLayoutManager.get();
	}

	VulkanFlightManager* VulkanContext::GetFlightManager()
	{
		return _flightManager.get();
	}

	void VulkanContext::RebuildSwapChain()
	{
		if (!_initialized)
			return;

		// Wait GPU finishing all tasks
		WaitDeviceIdle();

		// Create a new swap chain (auto release the old one)
		_pSwapChain = std::make_unique<VulkanSwapChain>();
	}

	VulkanPipelineManager* VulkanContext::GetPipelineManager()
	{
		return _pipelineManager.get();
	}

	bool VulkanContext::RenderFrame(bool* needRebuildSwapchain, const std::function<void(VulkanCommandBuffer*)>& recordCmdBufFunc)
	{
		// Fence frame context
		bool waitFinishSucc = _flightManager->WaitOneFlight();
		if (!waitFinishSucc)
			return false;

		// Acquire next image
		const vk::Semaphore imageReadySemaphore = _resourceManager->AllocateSemaphore();
		bool acquireImageSucc = _pSwapChain->AcquireNextImage(imageReadySemaphore, needRebuildSwapchain);
		if (!acquireImageSucc)
		{
			_resourceManager->FreeSemaphore(imageReadySemaphore, true);
			return false;
		}

		if (recordCmdBufFunc != nullptr)
			recordCmdBufFunc(_flightManager->GetRecordingCommandBuffer());

		// Submit command buffers
		return _flightManager->TakeOffFlight(_pSwapChain->GetCurrentImageIndex(), 
			imageReadySemaphore, needRebuildSwapchain);
	}

	void VulkanContext::WaitDeviceIdle()
	{
		// Fence all flinging frame -> Make sure tash all command buffers, semaphores
		// and fences are recycled.
		_flightManager->WaitAllFlight();

		// Wait gpu end
		_vkDevice.waitIdle();
	}

	VulkanSwapChain* VulkanContext::GetSwapChain()
	{
		return _pSwapChain.get();
	}

	void VulkanContext::PrepareDispatcher()
	{
		vk::detail::DynamicLoader loader;
		auto vkGetInstanceProcAddr =
			loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
	}

	void VulkanContext::CreateInstance(const GetWindowInstanceExtension& getWindowRequiredExtension, bool enableValidation)
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
		// Under macOS, the VK_KHR_portability_subset extension is required for portability, because
		// Metal does not fully support all Vulkan features.
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
		vk::DebugUtilsMessengerCreateInfoEXT createInfo;
		createInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
					  | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
			.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
				| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
				| vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
			.setPUserData(nullptr)
			.setPfnUserCallback(VulkanHelper::DebugCallback);

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

		// Extensions
		std::array extensions = {
			// Swap chain is required
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if AILURUS_PLATFORM_MAC
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
} // namespace Ailurus