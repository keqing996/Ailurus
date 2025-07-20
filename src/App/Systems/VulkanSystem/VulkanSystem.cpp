#include "VulkanSystem.h"
#include <memory>
#include <optional>
#include <array>
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/Descriptor/VulkanDescriptorPool.h"
#include "VulkanSystem/Helper/VulkanHelper.h"
#include "VulkanSystem/FrameContext/FrameContext.h"
#include "VulkanSystem/Resource/VulkanResourceManager.h"
#include "VulkanSystem/Vertex/VulkanVertexLayoutManager.h"
#include "VulkanSystem/Pipeline/VulkanPipelineManager.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Ailurus
{
	VulkanSystem::VulkanSystem(const GetWindowInstanceExtension& getWindowRequiredExtension,
		const WindowCreateSurfaceCallback& createSurface,
		const WindowDestroySurfaceCallback& destroySurface)
		: _destroySurfaceCallback(destroySurface)
		, _resourceManager(nullptr)
		, _vertexLayoutManager(nullptr)
		, _pipelineManager(nullptr)
	{
		PrepareDispatcher();

		VulkanHelper::LogInstanceLayerProperties();
		VulkanHelper::LogInstanceExtensionProperties();

		CreateInstance(getWindowRequiredExtension);
		CreatDebugUtilsMessenger();
		CreateSurface(createSurface);

		VulkanHelper::LogPhysicalCards(_vkInstance);

		ChoosePhysicsDevice();

		VulkanHelper::LogChosenPhysicalCard(_vkPhysicalDevice, _vkSurface);

		if (!CreateLogicalDevice())
			return;

		CreateCommandPool();

		_resourceManager = std::make_unique<VulkanResourceManager>();
		_vertexLayoutManager = std::make_unique<VulkanVertexLayoutManager>();
		_pipelineManager = std::make_unique<VulkanPipelineManager>();

		_initialized = true;
	}

	VulkanSystem::~VulkanSystem()
	{
		if (!_initialized)
			return;

		DestroyDynamicContext();

		_pipelineManager = nullptr;
		_vertexLayoutManager = nullptr;
		_resourceManager = nullptr;

		if (_vkDevice)
		{
			_vkDevice.destroyCommandPool(_vkGraphicCommandPool);
			_vkDevice.destroy();
		}

		if (_vkSurface)
			_destroySurfaceCallback(_vkInstance, _vkSurface);

		if (_vkDebugUtilsMessenger)
			_vkInstance.destroyDebugUtilsMessengerEXT(_vkDebugUtilsMessenger);

		if (_vkInstance)
			_vkInstance.destroy();

		_initialized = false;
	}

	bool VulkanSystem::Initialized() const
	{
		return _initialized;
	}



	uint32_t VulkanSystem::GetCurrentParallelFrameIndex() const
	{
		return _currentParallelFrameIndex;
	}

	void VulkanSystem::CreateSwapChain()
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

	void VulkanSystem::DestroySwapChain()
	{
		for (const auto& view : _vkSwapChainImageViews)
			_vkDevice.destroyImageView(view);

		_vkSwapChainImageViews.clear();

		if (_vkSwapChain != nullptr)
		{
			_vkDevice.destroySwapchainKHR(_vkSwapChain);
			_vkSwapChain = nullptr;
		}
	}

	const SwapChainConfig& VulkanSystem::GetSwapChainConfig() const
	{
		return _swapChainConfig;
	}

	const vk::SwapchainKHR& VulkanSystem::GetSwapChain() const
	{
		return _vkSwapChain;
	}

	const std::vector<vk::ImageView>& VulkanSystem::GetSwapChainImageViews()
	{
		return _vkSwapChainImageViews;
	}

	const FrameContext* VulkanSystem::GetFrameContext() const
	{
		return _frameContexts[_currentParallelFrameIndex].get();
	}

	VulkanResourceManager* VulkanSystem::GetResourceManager() const
	{
		return _resourceManager.get();
	}

	VulkanVertexLayoutManager* VulkanSystem::GetVertexLayoutManager() const
	{
		return _vertexLayoutManager.get();
	}

	VulkanPipelineManager* VulkanSystem::GetPipelineManager() const
	{
		return _pipelineManager.get();
	}

	FrameContext* VulkanSystem::GetFrameContext()
	{
		return _frameContexts[_currentParallelFrameIndex].get();
	}

	vk::CommandBuffer VulkanSystem::AllocateCommandBuffer()
	{
		return _commandBufferPool.Allocate();
	}

	void VulkanSystem::FreeCommandBuffer(vk::CommandBuffer commandBuffer)
	{
		_commandBufferPool.Free(commandBuffer);
	}

	vk::Semaphore VulkanSystem::AllocateSemaphore()
	{
		return _semaphorePool.Allocate();
	}

	void VulkanSystem::FreeSemaphore(vk::Semaphore semaphore)
	{
		_semaphorePool.Free(semaphore);
	}

	vk::Fence VulkanSystem::AllocateFence()
	{
		return _fencePool.Allocate();
	}

	void VulkanSystem::FreeFence(vk::Fence fence)
	{
		_fencePool.Free(fence);
	}
	
	std::unique_ptr<VulkanDescriptorPool> VulkanSystem::AllocateDescriptorPool()
	{
		return _descriptorPool.Allocate();
	}

	void VulkanSystem::FreeDescriptorPool(std::unique_ptr<VulkanDescriptorPool>&& pDescriptorPool)
	{
		_descriptorPool.Free(std::move(pDescriptorPool));
	}





	void VulkanSystem::CreateDynamicContext()
	{
		_currentParallelFrameIndex = 0;

		// Create swap chain
		CreateSwapChain();

		// Create frame context
		_frameContexts.clear();
		for (auto i = 0; i < PARALLEL_FRAME; i++)
		{
			_frameContexts.push_back(std::make_unique<FrameContext>());
			_frameContexts[i]->EnsureFrameInitialized();
		}
	}

	void VulkanSystem::DestroyDynamicContext()
	{
		if (!_initialized)
			return;

		WaitDeviceIdle();

		// Destroy frame context
		_frameContexts.clear();

		// Destroy the swap chain
		DestroySwapChain();
	}

	void VulkanSystem::WaitDeviceIdle() const
	{
		// Fence all flinging frame -> Make sure tash all command buffers, semaphores
		// and fences are recycled.
		for (auto& pFrameContext : _frameContexts)
			pFrameContext->WaitFinish();

		// Wait gpu end
		_vkDevice.waitIdle();
	}

	bool VulkanSystem::RenderFrame(bool* needRebuild)
	{
		FrameContext* pFrameContext = GetFrameContext();

		// Fence frame context
		bool waitFinishSucc = pFrameContext->WaitFinish();
		if (!waitFinishSucc)
			return false;

		// Acquire next image
		const vk::Semaphore imageReadySemaphore = AllocateSemaphore();

		auto acquireImage = _vkDevice.acquireNextImageKHR(_vkSwapChain,
			std::numeric_limits<uint64_t>::max(), imageReadySemaphore);

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
				_semaphorePool.Free(imageReadySemaphore, true);
				return false;
		}

		auto currentSwapChainImageIndex = acquireImage.value;

		// Submit command buffers
		const vk::Semaphore renderFinishSemaphore = pFrameContext->SubmitCommandBuffer(imageReadySemaphore);

		// Present
		vk::PresentInfoKHR presentInfo;
		presentInfo.setWaitSemaphores(renderFinishSemaphore)
			.setSwapchains(_vkSwapChain)
			.setImageIndices(currentSwapChainImageIndex);

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

		// Update flight index
		_currentParallelFrameIndex = (_currentParallelFrameIndex + 1) % PARALLEL_FRAME;

		return true;
	}

	
} // namespace Ailurus
