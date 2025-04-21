#include "RhiContext.h"
#include <unordered_set>
#include <mutex>
#include <optional>
#include <array>
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"

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

	// Validation
	bool RhiContext::enableValidation = true;

	// Init
	bool RhiContext::_initialized = false;

	// Static context
	vk::Instance RhiContext::_vkInstance = nullptr;
	vk::DebugUtilsMessengerEXT RhiContext::_vkDebugUtilsMessenger = nullptr;
	vk::PhysicalDevice RhiContext::_vkPhysicalDevice = nullptr;
	vk::SurfaceKHR RhiContext::_vkSurface = nullptr;
	vk::Device RhiContext::_vkDevice = nullptr;
	uint32_t RhiContext::_presentQueueIndex = 0;
	uint32_t RhiContext::_graphicQueueIndex = 0;
	uint32_t RhiContext::_computeQueueIndex = 0;
	vk::Queue RhiContext::_vkPresentQueue = nullptr;
	vk::Queue RhiContext::_vkGraphicQueue = nullptr;
	vk::Queue RhiContext::_vkComputeQueue = nullptr;
	vk::CommandPool RhiContext::_vkGraphicCommandPool = nullptr;

	// Dynamic context - swap chain
	SwapChainConfig RhiContext::_swapChainConfig{};
	vk::SwapchainKHR RhiContext::_vkSwapChain = nullptr;
	std::vector<vk::Image> RhiContext::_vkSwapChainImages{};
	std::vector<vk::ImageView> RhiContext::_vkSwapChainImageViews{};

	// Dynamic context - flight
	uint32_t RhiContext::_currentParallelFrameIndex = 0;
	unsigned RhiContext::_currentSwapChainImageIndex = 0;
	std::vector<vk::CommandBuffer> RhiContext::_vkCommandBuffers{};
	std::vector<vk::Semaphore> RhiContext::_vkImageReadySemaphore{};
	std::vector<vk::Semaphore> RhiContext::_vkFinishRenderSemaphore{};
	std::vector<vk::Fence> RhiContext::_vkFences{};

	bool RhiContext::Init(const GetWindowInstanceExtension& getWindowRequiredExtension, const WindowCreateSurfaceCallback& createSurface)
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

	void RhiContext::Destroy(const WindowDestroySurfaceCallback& destroySurface)
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

		if (_vkDebugUtilsMessenger)
			_vkInstance.destroyDebugUtilsMessengerEXT(_vkDebugUtilsMessenger);

		if (_vkInstance)
			_vkInstance.destroy();

		_initialized = false;
	}

	vk::Device RhiContext::GetDevice()
	{
		return _vkDevice;
	}

	vk::PhysicalDevice RhiContext::GetPhysicalDevice()
	{
		return _vkPhysicalDevice;
	}

	vk::SurfaceKHR RhiContext::GetSurface()
	{
		return _vkSurface;
	}

	uint32_t RhiContext::GetPresentQueueIndex()
	{
		return _presentQueueIndex;
	}

	uint32_t RhiContext::GetGraphicQueueIndex()
	{
		return _graphicQueueIndex;
	}

	vk::Queue RhiContext::GetPresentQueue()
	{
		return _vkPresentQueue;
	}

	vk::Queue RhiContext::GetGraphicQueue()
	{
		return _vkGraphicQueue;
	}

	uint32_t RhiContext::GetComputeQueueIndex()
	{
		return _computeQueueIndex;
	}

	vk::Queue RhiContext::GetComputeQueue()
	{
		return _vkComputeQueue;
	}

	vk::CommandPool RhiContext::GetCommandPool()
	{
		return _vkGraphicCommandPool;
	}

	uint32_t RhiContext::CurrentParallelFrameIndex()
	{
		return _currentParallelFrameIndex;
	}

	void RhiContext::RebuildDynamicContext()
	{
		if (!_initialized)
			return;

		_vkDevice.waitIdle();

		DestroyDynamicContext();
		CreateDynamicContext();
	}

	void RhiContext::CreateSwapChain()
	{
		// Present mode
		auto allPresentMode = _vkPhysicalDevice.getSurfacePresentModesKHR(_vkSurface);
		for (const vk::PresentModeKHR& mode : allPresentMode)
		{
			if (mode == vk::PresentModeKHR::eMailbox)
			{
				_swapChainConfig.presentMode = mode;
				break;
			}
		}

		// Format
		auto surfaceFormats = _vkPhysicalDevice.getSurfaceFormatsKHR(_vkSurface);
		for (auto& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == vk::Format::eR8G8B8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				_swapChainConfig.surfaceFormat = surfaceFormat;
				break;
			}

			if (surfaceFormat.format == vk::Format::eB8G8R8A8Unorm && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				_swapChainConfig.surfaceFormat = surfaceFormat;
				break;
			}
		}

		if (_swapChainConfig.surfaceFormat.format == vk::Format::eUndefined)
			Logger::LogError("No suitable surface format (format R8G8B8SRGB & colorspace SRGB non-linear)");

		// Swap chain image count & size
		Vector2i windowSize = Application::GetSize();
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = _vkPhysicalDevice.getSurfaceCapabilitiesKHR(_vkSurface);
		_swapChainConfig.imageCount = std::clamp(2u, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
		_swapChainConfig.extent.width = std::clamp(static_cast<uint32_t>(windowSize.x),
			surfaceCapabilities.minImageExtent.width,
			surfaceCapabilities.maxImageExtent.width);
		_swapChainConfig.extent.height = std::clamp(static_cast<uint32_t>(windowSize.y),
			surfaceCapabilities.minImageExtent.height,
			surfaceCapabilities.maxImageExtent.height);

		// Create
		vk::SwapchainCreateInfoKHR swapChainCreateInfo;
		swapChainCreateInfo
			.setSurface(_vkSurface) // target surface
			.setImageArrayLayers(1) // not cube image
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setClipped(true)										   // clipped when image's pixel outside of window
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque) // No alpha blending when image output to window
			.setMinImageCount(_swapChainConfig.imageCount)
			.setImageFormat(_swapChainConfig.surfaceFormat.format)
			.setImageColorSpace(_swapChainConfig.surfaceFormat.colorSpace)
			.setImageExtent(_swapChainConfig.extent);

		if (_graphicQueueIndex == _presentQueueIndex)
		{
			swapChainCreateInfo
				.setImageSharingMode(vk::SharingMode::eExclusive)
				.setQueueFamilyIndices(_graphicQueueIndex);
		}
		else
		{
			std::array indices = { _graphicQueueIndex, _presentQueueIndex };
			swapChainCreateInfo
				.setImageSharingMode(vk::SharingMode::eConcurrent)
				.setQueueFamilyIndices(indices);
		}

		_vkSwapChain = _vkDevice.createSwapchainKHR(swapChainCreateInfo);

		// Swap chain image & view
		_vkSwapChainImages = _vkDevice.getSwapchainImagesKHR(_vkSwapChain);
		_vkSwapChainImageViews.resize(_vkSwapChainImages.size());
		for (auto i = 0; i < _vkSwapChainImages.size(); i++)
		{
			vk::ImageSubresourceRange range;
			range.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)	  // first mipmap level accessible to the view
				.setLevelCount(1)	  // number of mipmap levels (starting from baseMipLevel) accessible to the view
				.setBaseArrayLayer(0) // first array layer accessible to the view
				.setLayerCount(1);	  // number of array layers (starting from baseArrayLayer) accessible to the view

			vk::ImageViewCreateInfo imageViewCreateInfo;
			imageViewCreateInfo
				.setImage(_vkSwapChainImages[i])
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(_swapChainConfig.surfaceFormat.format)
				.setComponents(vk::ComponentMapping())
				.setSubresourceRange(range);

			_vkSwapChainImageViews[i] = _vkDevice.createImageView(imageViewCreateInfo);
		}
	}

	void RhiContext::DestroySwapChain()
	{
		for (const auto& view : _vkSwapChainImageViews)
			_vkDevice.destroyImageView(view);

		_vkDevice.destroySwapchainKHR(_vkSwapChain);
	}

	void RhiContext::CreateCommandBuffer()
	{
		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.setCommandPool(_vkGraphicCommandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(PARALLEL_FRAME);

		_vkCommandBuffers = _vkDevice.allocateCommandBuffers(allocInfo);
	}

	void RhiContext::DestroyCommandBuffer()
	{
		_vkDevice.freeCommandBuffers(RhiContext::GetCommandPool(), _vkCommandBuffers);
	}

	void RhiContext::CreateSynchronizationObjects()
	{
		vk::SemaphoreCreateInfo semaphoreInfo;

		vk::FenceCreateInfo fenceInfo;
		fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

		for (size_t i = 0; i < PARALLEL_FRAME; i++)
		{
			_vkImageReadySemaphore.push_back(RhiContext::GetDevice().createSemaphore(semaphoreInfo));
			_vkFinishRenderSemaphore.push_back(RhiContext::GetDevice().createSemaphore(semaphoreInfo));
			_vkFences.push_back(RhiContext::GetDevice().createFence(fenceInfo));
		}
	}

	void RhiContext::DestroySynchronizationObjects()
	{
		for (const auto& fence : _vkFences)
			_vkDevice.destroyFence(fence);

		for (const auto& sem : _vkImageReadySemaphore)
			_vkDevice.destroySemaphore(sem);

		for (const auto& sem : _vkFinishRenderSemaphore)
			_vkDevice.destroySemaphore(sem);
	}

	const SwapChainConfig& RhiContext::GetSwapChainConfig()
	{
		return _swapChainConfig;
	}

	const vk::SwapchainKHR& RhiContext::GetSwapChain()
	{
		return _vkSwapChain;
	}

	const std::vector<vk::ImageView>& RhiContext::GetSwapChainImageViews()
	{
		return _vkSwapChainImageViews;
	}

	uint32_t RhiContext::GetCurrentFrameIndex()
	{
		return _currentParallelFrameIndex;
	}

	const vk::CommandBuffer& RhiContext::GetCurrentFrameCommandBuffer()
	{
		return _vkCommandBuffers[_currentParallelFrameIndex];
	}

	const vk::Semaphore& RhiContext::GetCurrentFrameImageReadySemaphore()
	{
		return _vkImageReadySemaphore[_currentParallelFrameIndex];
	}

	const vk::Semaphore& RhiContext::GetCurrentFrameRenderFinishSemaphore()
	{
		return _vkFinishRenderSemaphore[_currentParallelFrameIndex];
	}

	const vk::Fence& RhiContext::GetCurrentFrameFence()
	{
		return _vkFences[_currentParallelFrameIndex];
	}

	bool RhiContext::WaitNextFrame(bool* needRebuild)
	{
		// Wait fence
		auto waitFence = _vkDevice.waitForFences(GetCurrentFrameFence(), true, std::numeric_limits<uint64_t>::max());
		if (waitFence != vk::Result::eSuccess)
		{
			Logger::LogError("Fail to wait fences, result = {}", static_cast<int>(waitFence));
			return false;
		}

		// Acquire swap chain next image
		auto acquireImage = _vkDevice.acquireNextImageKHR(_vkSwapChain,
			std::numeric_limits<uint64_t>::max(), GetCurrentFrameImageReadySemaphore());

		switch (acquireImage.result)
		{
			case vk::Result::eErrorOutOfDateKHR:
				*needRebuild = true;
				return false;
			case vk::Result::eSuboptimalKHR:
				*needRebuild = true;
				break;
			case vk::Result::eSuccess:
				break;
			default:
				Logger::LogError("Fail to acquire next image, result = {}", static_cast<int>(acquireImage.result));
				return false;
		}

		_currentSwapChainImageIndex = acquireImage.value;

		_vkDevice.resetFences(GetCurrentFrameFence());

		return true;
	}

	bool RhiContext::SubmitThisFrame(bool* needRebuild)
	{
		std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

		vk::SubmitInfo submitInfo;
		submitInfo.setWaitSemaphores(GetCurrentFrameImageReadySemaphore())
				.setSignalSemaphores(GetCurrentFrameRenderFinishSemaphore())
				.setWaitDstStageMask(waitStages)
				.setCommandBuffers(GetCurrentFrameCommandBuffer());

		_vkGraphicQueue.submit(submitInfo, GetCurrentFrameFence());

		vk::PresentInfoKHR presentInfo;
		presentInfo.setWaitSemaphores(GetCurrentFrameRenderFinishSemaphore())
				.setSwapchains(_vkSwapChain)
				.setImageIndices(_currentSwapChainImageIndex);

		switch (const auto present = _vkPresentQueue.presentKHR(presentInfo))
		{
			case vk::Result::eErrorOutOfDateKHR:
				*needRebuild = true;
				return false;
			case vk::Result::eSuboptimalKHR:
				*needRebuild = true;
				break;
			case vk::Result::eSuccess:
				break;
			default:
				Logger::LogError("Fail to present, result = {}", static_cast<int>(present));
				return false;
		}

		_currentParallelFrameIndex = (_currentParallelFrameIndex + 1) % PARALLEL_FRAME;

		return true;
	}

	void RhiContext::PrepareDispatcher()
	{
		vk::detail::DynamicLoader loader;
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
			loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
	}

	void RhiContext::CreateInstance(const GetWindowInstanceExtension& getWindowRequiredExtension)
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
		// Metal do not fully support all Vulkan features.
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

	void RhiContext::CreatDebugUtilsMessenger()
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

	void RhiContext::CreateSurface(const WindowCreateSurfaceCallback& createSurface)
	{
		_vkSurface = createSurface(_vkInstance);
	}

	void RhiContext::ChoosePhysicsDevice()
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

	bool RhiContext::CreateLogicalDevice()
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

	void RhiContext::CreateCommandPool()
	{
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(GetGraphicQueueIndex());

		_vkGraphicCommandPool = _vkDevice.createCommandPool(poolInfo);
	}

	void RhiContext::CreateDynamicContext()
	{
		CreateSwapChain();

		_currentParallelFrameIndex = 0;
		_currentSwapChainImageIndex = 0;

		CreateCommandBuffer();
		CreateSynchronizationObjects();
	}

	void RhiContext::DestroyDynamicContext()
	{
		DestroySynchronizationObjects();
		DestroyCommandBuffer();
		DestroySwapChain();
	}
} // namespace Ailurus
