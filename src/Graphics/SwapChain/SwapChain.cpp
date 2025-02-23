#include "Ailurus/Graphics/SwapChain/SwapChain.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Graphics/Renderer.h"

namespace Ailurus
{
    SwapChain::SwapChain(const Renderer* pRenderer, int windowWidth, int windowHeight)
        : _pRenderer(pRenderer)
    {
        auto vkPhysicalDevice = _pRenderer->GetPhysicalDevice();
        auto vkSurface = _pRenderer->GetSurface();
        auto vkLogicalDevice = _pRenderer->GetLogicalDevice();
        auto graphicQueueIndex = _pRenderer->GetGraphicQueueIndex();
        auto presentQueueIndex = _pRenderer->GetPresentQueueIndex();

        // Present mode
        auto allPresentMode = vkPhysicalDevice.getSurfacePresentModesKHR(vkSurface);
        for (const vk::PresentModeKHR& mode : allPresentMode)
        {
            if (mode == vk::PresentModeKHR::eMailbox)
            {
                _swapChainConfig.presentMode = mode;
                break;
            }
        }

        // Format
        auto surfaceFormats = vkPhysicalDevice.getSurfaceFormatsKHR(vkSurface);
        for (auto& surfaceFormat: surfaceFormats)
        {
            if (surfaceFormat.format == vk::Format::eR8G8B8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                _swapChainConfig.surfaceFormat = surfaceFormat;
                break;
            }
        }

        if (_swapChainConfig.surfaceFormat == vk::Format::eUndefined)
            Logger::LogError("No suitable surface format (format R8G8B8SRGB & colorspace SRGB non-linear)");


        // Swap chain image count & size
        vk::SurfaceCapabilitiesKHR surfaceCapabilities = vkPhysicalDevice.getSurfaceCapabilitiesKHR(vkSurface);
        _swapChainConfig.imageCount = std::clamp(2u, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
        _swapChainConfig.extent.width  = std::clamp(static_cast<uint32_t>(windowWidth),
                                           surfaceCapabilities.minImageExtent.width,
                                           surfaceCapabilities.maxImageExtent.width);
        _swapChainConfig.extent.height = std::clamp(static_cast<uint32_t>(windowHeight),
                                            surfaceCapabilities.minImageExtent.height,
                                            surfaceCapabilities.maxImageExtent.height);


        // Create
        vk::SwapchainCreateInfoKHR swapChainCreateInfo;
        swapChainCreateInfo
            .setSurface(vkSurface) // target surface
            .setImageArrayLayers(1) // not cube image
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setClipped(true)   // clipped when image's pixel outside of window
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)  // No alpha blending when image output to window
            .setMinImageCount(_swapChainConfig.imageCount)
            .setImageFormat(_swapChainConfig.surfaceFormat.format)
            .setImageColorSpace(_swapChainConfig.surfaceFormat.colorSpace)
            .setImageExtent(_swapChainConfig.extent);

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

        _vkSwapChain = vkLogicalDevice.createSwapchainKHR(swapChainCreateInfo);

        // Swap chain image & view
        _vkSwapChainImages = vkLogicalDevice.getSwapchainImagesKHR(_vkSwapChain);
        _vkSwapChainImageViews.resize(_vkSwapChainImages.size());
        for (auto i = 0; i < _vkSwapChainImages.size(); i++)
        {
            vk::ImageSubresourceRange range;
            range.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0) // first mipmap level accessible to the view
                .setLevelCount(1)   // number of mipmap levels (starting from baseMipLevel) accessible to the view
                .setBaseArrayLayer(0) // first array layer accessible to the view
                .setLayerCount(1); //number of array layers (starting from baseArrayLayer) accessible to the view

            vk::ImageViewCreateInfo imageViewCreateInfo;
            imageViewCreateInfo
                .setImage(_vkSwapChainImages[i])
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(_swapChainConfig.surfaceFormat.format)
                .setComponents(vk::ComponentMapping())
                .setSubresourceRange(range);

            _vkSwapChainImageViews[i] = vkLogicalDevice.createImageView(imageViewCreateInfo);
        }
    }

    SwapChain::~SwapChain()
    {
        for (auto& view : _vkSwapChainImageViews)
            _pRenderer->GetLogicalDevice().destroyImageView(view);

        _pRenderer->GetLogicalDevice().destroySwapchainKHR(_vkSwapChain);
    }

    const SwapChainConfig& SwapChain::GetSwapChainConfig() const
    {
        return _swapChainConfig;
    }

    const vk::SwapchainKHR& SwapChain::GetSwapChain() const
    {
        return _vkSwapChain;
    }

    const std::vector<vk::Image>& SwapChain::GetImages() const
    {
        return _vkSwapChainImages;
    }

    const std::vector<vk::ImageView>& SwapChain::GetImageViews() const
    {
        return _vkSwapChainImageViews;
    }
}
