
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
		try
		{
			auto allPresentMode = VulkanContext::GetPhysicalDevice().getSurfacePresentModesKHR(VulkanContext::GetSurface());
			for (const vk::PresentModeKHR& mode : allPresentMode)
			{
				if (mode == vk::PresentModeKHR::eMailbox)
				{
					_swapChainConfig.presentMode = mode;
					break;
				}
			}
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to get surface present modes: {}", e.what());
		}

		// Format
		try
		{
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
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to get surface formats: {}", e.what());
		}

		if (_swapChainConfig.surfaceFormat.format == vk::Format::eUndefined)
			Logger::LogError("No suitable surface format (format R8G8B8SRGB & colorspace SRGB non-linear)");

		// Swap chain image count & size
		Vector2i windowSize = Application::GetDrawableSize();
		try
		{
			vk::SurfaceCapabilitiesKHR surfaceCapabilities = VulkanContext::GetPhysicalDevice().getSurfaceCapabilitiesKHR(VulkanContext::GetSurface());
			_swapChainConfig.imageCount = std::clamp(2u, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
			_swapChainConfig.extent.width = std::clamp(static_cast<uint32_t>(windowSize.x),
				surfaceCapabilities.minImageExtent.width,
				surfaceCapabilities.maxImageExtent.width);
			_swapChainConfig.extent.height = std::clamp(static_cast<uint32_t>(windowSize.y),
				surfaceCapabilities.minImageExtent.height,
				surfaceCapabilities.maxImageExtent.height);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to get surface capabilities: {}", e.what());
		}

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

		try
		{
			_vkSwapChain = VulkanContext::GetDevice().createSwapchainKHR(swapChainCreateInfo);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create swapchain: {}", e.what());
		}

		// Swap chain image & view
		try
		{
			_vkSwapChainImages = VulkanContext::GetDevice().getSwapchainImagesKHR(_vkSwapChain);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to get swapchain images: {}", e.what());
		}
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

			try
			{
				_vkSwapChainImageViews[i] = VulkanContext::GetDevice().createImageView(imageViewCreateInfo);
			}
			catch (const vk::SystemError& e)
			{
				Logger::LogError("Failed to create image view: {}", e.what());
			}
		}
	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		try
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
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to destroy swapchain or image views: {}", e.what());
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

	std::optional<uint32_t> VulkanSwapChain::AcquireNextImage(VulkanSemaphore* imageReadySem, bool* needRebuildSwapChain)
	{
		try
		{
			vk::ResultValue<uint32_t> acquireResult = VulkanContext::GetDevice().acquireNextImageKHR(
				_vkSwapChain,
				std::numeric_limits<uint64_t>::max(),
				imageReadySem->GetSemaphore());

			if (acquireResult.result == vk::Result::eSuboptimalKHR)
				*needRebuildSwapChain = true;

			return acquireResult.value;
		}
		catch (const vk::OutOfDateKHRError& e)
		{
			Logger::LogError("Swap chain out of date: {}", e.what());
			*needRebuildSwapChain = true;
			return std::nullopt;
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to acquire next image: {}", e.what());
			return std::nullopt;
		}
	}
} // namespace Ailurus