#include <memory>
#include <optional>
#include <array>
#include "VulkanContext.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"
#include "SwapChain/VulkanSwapChain.h"
#include "Helper/VulkanHelper.h"
#include "Resource/VulkanResourceManager.h"
#include "Vertex/VulkanVertexLayoutManager.h"
#include "Pipeline/VulkanPipelineManager.h"
#include "Fence/VulkanFence.h"
#include "FrameBuffer/VulkanFrameBufferManager.h"
#include "Descriptor/VulkanDescriptorAllocator.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Ailurus
{
	uint32_t									VulkanContext::_parallelFrameCount = 2;
	bool 										VulkanContext::_initialized = false;

	vk::Instance 								VulkanContext::_vkInstance = nullptr;
	vk::DebugUtilsMessengerEXT 					VulkanContext::_vkDebugUtilsMessenger = nullptr;
	vk::PhysicalDevice 							VulkanContext::_vkPhysicalDevice = nullptr;
	vk::SurfaceKHR 								VulkanContext::_vkSurface = nullptr;
	vk::Device 									VulkanContext::_vkDevice = nullptr;
	uint32_t 									VulkanContext::_presentQueueIndex = 0;
	uint32_t 									VulkanContext::_graphicQueueIndex = 0;
	uint32_t 									VulkanContext::_computeQueueIndex = 0;
	vk::Queue 									VulkanContext::_vkPresentQueue = nullptr;
	vk::Queue 									VulkanContext::_vkGraphicQueue = nullptr;
	vk::Queue 									VulkanContext::_vkComputeQueue = nullptr;
	vk::CommandPool 							VulkanContext::_vkGraphicCommandPool = nullptr;

	std::unique_ptr<VulkanSwapChain> 			VulkanContext::_pSwapChain = nullptr;

	std::unique_ptr<VulkanResourceManager> 		VulkanContext::_resourceManager = nullptr;
	std::unique_ptr<VulkanVertexLayoutManager> 	VulkanContext::_vertexLayoutManager = nullptr;
	std::unique_ptr<VulkanPipelineManager> 		VulkanContext::_pipelineManager = nullptr;
	std::unique_ptr<VulkanFrameBufferManager> 	VulkanContext::_frameBufferManager = nullptr;

	uint32_t									VulkanContext::_currentFrameIndex = 0;
	std::vector<VulkanContext::FrameContext>	VulkanContext::_frameContext;

	std::vector<std::unique_ptr<VulkanCommandBuffer>>	VulkanContext::_recordedSecondaryCommandBuffers;
	std::vector<std::unique_ptr<VulkanCommandBuffer>>	VulkanContext::_secondaryCommandBufferPool;

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
		_frameBufferManager = std::make_unique<VulkanFrameBufferManager>();

		// Create frame context
		_currentFrameIndex = 0;
		for (auto i = 0; i < _parallelFrameCount; ++i)
		{
			FrameContext frameContext;
			frameContext.onAirInfo = std::nullopt; // Not in flight
			frameContext.pRenderingCommandBuffer = std::make_unique<VulkanCommandBuffer>(true);
			frameContext.pFrameDescriptorAllocator = std::make_unique<VulkanDescriptorAllocator>();
			frameContext.imageReadySemaphore = std::make_unique<VulkanSemaphore>();
			frameContext.renderFinishSemaphore = std::make_unique<VulkanSemaphore>();
			frameContext.renderFinishFence = std::make_unique<VulkanFence>(false);
			_frameContext.push_back(std::move(frameContext));
		}

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

		WaitDeviceIdle();

		// Destroy frame context
		_recordedSecondaryCommandBuffers.clear();
		_secondaryCommandBufferPool.clear();
		_frameContext.clear();
		_currentFrameIndex = 0;

		// Destroy managers
		_frameBufferManager.reset();
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

	VulkanFrameBufferManager* VulkanContext::GetFrameBufferManager()
	{
		return _frameBufferManager.get();
	}

	uint32_t VulkanContext::GetParallelFrameCount()
	{
		return _parallelFrameCount;
	}

	void VulkanContext::RebuildSwapChain()
	{
		if (!_initialized)
			return;

		// Wait GPU finishing all tasks
		WaitDeviceIdle();

		// Clear all back buffers
		_frameBufferManager->ClearBackBuffers();

		// Release old swap chian fist
		_pSwapChain = nullptr;

		// Create a new swap chain
		_pSwapChain = std::make_unique<VulkanSwapChain>();
	}

	void VulkanContext::RecordSecondaryCommandBuffer(const RecordSecondaryCommandBufferFunction& recordFunction)
	{
		if (recordFunction == nullptr)
			return;

		std::unique_ptr<VulkanCommandBuffer> pSecondaryCommandBuffer = nullptr;
		if (!_secondaryCommandBufferPool.empty())
		{
			pSecondaryCommandBuffer = std::move(_secondaryCommandBufferPool.back());
			_secondaryCommandBufferPool.pop_back();
		}

		if (pSecondaryCommandBuffer == nullptr)
			pSecondaryCommandBuffer = std::make_unique<VulkanCommandBuffer>(false);

		pSecondaryCommandBuffer->Begin();
		recordFunction(pSecondaryCommandBuffer.get());
		pSecondaryCommandBuffer->End();

		_recordedSecondaryCommandBuffers.push_back(std::move(pSecondaryCommandBuffer));
	}

	VulkanPipelineManager* VulkanContext::GetPipelineManager()
	{
		return _pipelineManager.get();
	}

	bool VulkanContext::RenderFrame(bool* needRebuildSwapChain, const RenderFunction& recordCmdBufFunc)
	{
		// Fence frame context
		WaitFrameFinish(_currentFrameIndex);
		auto& frameContext = _frameContext[_currentFrameIndex];

		// Acquire next image
		//  - Image ready semaphore will **NOT** be signaled when the result of AcquireNextImageKHR is not eSuccess
		//    or eSuboptimalKHR, so it is safe to recycle the semaphore.
		const auto opImageIndex = _pSwapChain->AcquireNextImage(frameContext.imageReadySemaphore.get(), needRebuildSwapChain);
		if (!opImageIndex.has_value())
			return false;

		const auto imageIndex = opImageIndex.value();

		// Record
		frameContext.pRenderingCommandBuffer->Begin();
		{
			// Record secondary
			for (auto& pSecondaryCmdBuffer : _recordedSecondaryCommandBuffers)
				frameContext.pRenderingCommandBuffer->ExecuteSecondaryCommandBuffer(pSecondaryCmdBuffer.get());

			// Render
			if (recordCmdBufFunc != nullptr)
				recordCmdBufFunc(imageIndex, frameContext.pRenderingCommandBuffer.get(), frameContext.pFrameDescriptorAllocator.get());
		}
		frameContext.pRenderingCommandBuffer->End();

		// Do submit
		vk::SubmitInfo submitInfo;
		submitInfo.setCommandBuffers(frameContext.pRenderingCommandBuffer->GetBuffer())
			.setSignalSemaphores(frameContext.renderFinishSemaphore->GetSemaphore())
			.setWaitSemaphores(frameContext.imageReadySemaphore->GetSemaphore());

		// Wait stages
		std::vector<vk::PipelineStageFlags> waitStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.setWaitDstStageMask(waitStages);

		// Submit
		_vkGraphicQueue.submit(submitInfo, frameContext.renderFinishFence->GetFence());

		// Set this frame on air
		frameContext.onAirInfo = OnAirInfo{
			.frameCount = Application::Get<TimeSystem>()->FrameCount(),
			.secondaryCommandBuffers = std::move(_recordedSecondaryCommandBuffers)
		};

		// Present
		vk::PresentInfoKHR presentInfo;
		presentInfo.setWaitSemaphores(frameContext.renderFinishSemaphore->GetSemaphore())
			.setSwapchains(_pSwapChain->GetSwapChain())
			.setImageIndices(imageIndex);

		bool presentResult = false;
		switch (const auto present = _vkPresentQueue.presentKHR(presentInfo))
		{
			case vk::Result::eSuccess:
				presentResult = true;
				break;
			case vk::Result::eSuboptimalKHR:
				*needRebuildSwapChain = true;
				presentResult = true;
				break;
			case vk::Result::eErrorOutOfDateKHR:
				Logger::LogError("Fail to present, error out of date");
				*needRebuildSwapChain = true;
				presentResult = false;
				break;
			default:
				Logger::LogError("Fail to present, result = {}", static_cast<int>(present));
				presentResult = false;
				break;
		}

		// Update frame index
		_currentFrameIndex = (_currentFrameIndex + 1) % _parallelFrameCount;

		return presentResult;
	}

	void VulkanContext::WaitDeviceIdle()
	{
		// Fence all flinging frame -> Make sure tash all command buffers, semaphores
		// and fences are recycled.
		for (auto i = 0; i < _frameContext.size(); i++)
			WaitFrameFinish(i);

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

	bool VulkanContext::WaitFrameFinish(uint32_t index)
	{
		auto& context = _frameContext[index];

		// Not in flight
		if (!context.onAirInfo.has_value())
			return true;

		// Wait render finish fence
		const auto waitFence = _vkDevice.waitForFences(context.renderFinishFence->GetFence(),
			true, std::numeric_limits<uint64_t>::max());
		if (waitFence != vk::Result::eSuccess)
		{
			Logger::LogError("Fail to wait fences, result = {}", static_cast<int>(waitFence));
			return false;
		}

		// Reset frame resource
		context.renderFinishFence->Reset();
		context.pRenderingCommandBuffer->ClearResourceReferences();
		context.pFrameDescriptorAllocator->ResetPool();

		// Recycle secondary command buffers
		if (!context.onAirInfo->secondaryCommandBuffers.empty())
		{
			auto& vec = context.onAirInfo->secondaryCommandBuffers;
			for (auto i = 0; i < vec.size(); i++)
			{
				vec[i]->ClearResourceReferences();
				_secondaryCommandBufferPool.push_back(std::move(vec[i]));
			}
		}

		// Reset on air info
		context.onAirInfo = std::nullopt;

		// Resource GC
		_resourceManager->GarbageCollect();

		return true;
	}
} // namespace Ailurus