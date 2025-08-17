
#include "VulkanSwapChain.h"
#include "Ailurus/Math/Vector2.hpp"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanContext/VulkanContext.h"

namespace Ailurus
{
	VulkanSwapChain::VulkanSwapChain()
	{
		// Present mode
		auto allPresentMode = VulkanContext::GetPhysicalDevice().getSurfacePresentModesKHR(VulkanContext::GetSurface());
		for (const vk::PresentModeKHR& mode : allPresentMode)
		{
			if (mode == vk::PresentModeKHR::eMailbox)
			{
				_swapChainConfig.presentMode = mode;
				break;
			}
		}

		// Format
		auto surfaceFormats = VulkanContext::GetPhysicalDevice().getSurfaceFormatsKHR(VulkanContext::GetSurface());
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
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = VulkanContext::GetPhysicalDevice().getSurfaceCapabilitiesKHR(VulkanContext::GetSurface());
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
			.setSurface(VulkanContext::GetSurface()) // target surface
			.setImageArrayLayers(1)					 // not cube image
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setClipped(true)										   // clipped when image's pixel outside of window
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque) // No alpha blending when image output to window
			.setMinImageCount(_swapChainConfig.imageCount)
			.setImageFormat(_swapChainConfig.surfaceFormat.format)
			.setImageColorSpace(_swapChainConfig.surfaceFormat.colorSpace)
			.setImageExtent(_swapChainConfig.extent);

		const uint32_t graphicQueueIndex = VulkanContext::GetGraphicQueueIndex();
		const uint32_t presentQueueIndex = VulkanContext::GetPresentQueueIndex();

		if (graphicQueueIndex == presentQueueIndex)
		{
			swapChainCreateInfo
				.setImageSharingMode(vk::SharingMode::eExclusive)
				.setQueueFamilyIndices(graphicQueueIndex);
		}
		else
		{
			std::array indices = { graphicQueueIndex, presentQueueIndex };
			swapChainCreateInfo
				.setImageSharingMode(vk::SharingMode::eConcurrent)
				.setQueueFamilyIndices(indices);
		}

		_vkSwapChain = VulkanContext::GetDevice().createSwapchainKHR(swapChainCreateInfo);

		// Swap chain image & view
		_vkSwapChainImages = VulkanContext::GetDevice().getSwapchainImagesKHR(_vkSwapChain);
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

			_vkSwapChainImageViews[i] = VulkanContext::GetDevice().createImageView(imageViewCreateInfo);
		}

		// Frame context
		for (auto i = 0; i < _vkSwapChainImages.size(); i++)
		{
			auto pFrameContext = std::make_unique<FrameContext>();
			pFrameContext->frameCount = 0;
			pFrameContext->pRenderingCommandBuffer = std::make_unique<VulkanCommandBuffer>();
			pFrameContext->pFrameDescriptorAllocator = std::make_unique<VulkanDescriptorAllocator>();
			pFrameContext->pImageReadySemaphore = std::make_unique<VulkanSemaphore>();
			pFrameContext->pRenderFinishedSemaphore = std::make_unique<VulkanSemaphore>();
			pFrameContext->pRenderFinishFence = std::make_unique<VulkanFence>();
			_frameContext.push_back(std::move(pFrameContext));
		}
	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		for (const auto& view : _vkSwapChainImageViews)
			VulkanContext::GetDevice().destroyImageView(view);

		_vkSwapChainImageViews.clear();

		if (_vkSwapChain != nullptr)
		{
			VulkanContext::GetDevice().destroySwapchainKHR(_vkSwapChain);
			_vkSwapChain = nullptr;
		}
	}

	const SwapChainConfig& VulkanSwapChain::GetConfig() const
	{
		return _swapChainConfig;
	}

	const vk::SwapchainKHR& VulkanSwapChain::GetSwapChain() const
	{
		return _vkSwapChain;
	}

	const std::vector<vk::Image>& VulkanSwapChain::GetSwapChainImages() const
	{
		return _vkSwapChainImages;
	}

	const std::vector<vk::ImageView>& VulkanSwapChain::GetSwapChainImageViews() const
	{
		return _vkSwapChainImageViews;
	}

	uint32_t VulkanSwapChain::GetCurrentImageIndex() const
	{
		return _currentImageIndex;
	}

	std::unique_ptr<VulkanSwapChain::FrameSyncObjects> VulkanSwapChain::AcquireNextImage(bool* needRebuildSwapChain)
	{
		auto pFrameSyncObjects = AllocateFrameSyncObjects();

		auto acquireImage = VulkanContext::GetDevice().acquireNextImageKHR(
			_vkSwapChain,
			std::numeric_limits<uint64_t>::max(), 
			pFrameSyncObjects->pImageReadySemaphore->GetSemaphore());

		bool success = true;
		switch (acquireImage.result)
		{
			case vk::Result::eSuccess:
				break;
			// Swapchain can still work but is not perfect, rebuild swapchain next frame.
			case vk::Result::eSuboptimalKHR:
				*needRebuildSwapChain = true;
				break;
			// Swapchain is not fit for current surface, must be rebuilded.
			case vk::Result::eErrorOutOfDateKHR:
				*needRebuildSwapChain = true;
				success = false;
			default:
				Logger::LogError("Fail to acquire next image, result = {}", static_cast<int>(acquireImage.result));
				success = false;
		}

		if (!success)
		{
			// Image ready semaphore will **NOT** be signaled when the result of AcquireNextImageKHR is not eSuccess
			// or eSuboptimalKHR, so it is safe to recycle the semaphore.
			RecycleFrameSyncObjects(std::move(pFrameSyncObjects));
			return nullptr;
		}

		_currentImageIndex = acquireImage.value;
		return pFrameSyncObjects;
	}

	void VulkanSwapChain::RecycleFrameSyncObjects(std::unique_ptr<FrameSyncObjects>&& pFrameSyncObjects)
	{
		_frameSyncObjectsQueue.push_back(std::move(pFrameSyncObjects));
	}

	std::unique_ptr<VulkanSwapChain::FrameSyncObjects> VulkanSwapChain::AllocateFrameSyncObjects()
	{
		if (!_frameSyncObjectsQueue.empty())
		{
			auto result = std::move(_frameSyncObjectsQueue.front());
			_frameSyncObjectsQueue.pop_front();
			return result;
		}
		else 
		{
			auto pFrameSyncObjects = std::make_unique<FrameSyncObjects>();
			pFrameSyncObjects->pImageReadySemaphore = std::make_unique<VulkanSemaphore>();
			pFrameSyncObjects->pRenderFinishedSemaphore = std::make_unique<VulkanSemaphore>();
			pFrameSyncObjects->pRenderFinishFence = std::make_unique<VulkanFence>();
			return pFrameSyncObjects;
		}
	}
} // namespace Ailurus