#include "Ailurus/Graphics/SwapChain/SwapChain.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    SwapChain::~SwapChain()
    {
        _pContext->GetLogicalDevice().destroySwapchainKHR(_vkSwapChain);
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

    std::unique_ptr<SwapChain> SwapChain::Create(const VulkanContext* pContext, int windowWidth, int windowHeight)
    {
        auto vkPhysicalDevice = pContext->GetPhysicalDevice();
        auto vkSurface = pContext->GetSurface();
        SwapChainConfig config;

        // Present mode
        auto allPresentMode = vkPhysicalDevice.getSurfacePresentModesKHR(vkSurface);
        for (const vk::PresentModeKHR& mode : allPresentMode)
        {
            if (mode == vk::PresentModeKHR::eMailbox)
            {
                config.presentMode = mode;
                break;
            }
        }

        // Format
        auto surfaceFormats = vkPhysicalDevice.getSurfaceFormatsKHR(vkSurface);
        for (auto& surfaceFormat: surfaceFormats)
        {
            if (surfaceFormat.format == vk::Format::eR8G8B8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                config.surfaceFormat = surfaceFormat;
                break;
            }
        }

        if (config.surfaceFormat == vk::Format::eUndefined)
        {
            Logger::LogError("No suitable surface format (format R8G8B8SRGB & colorspace SRGB non-linear)");
            return nullptr;
        }

        // Swap chain image count & size
        vk::SurfaceCapabilitiesKHR surfaceCapabilities = vkPhysicalDevice.getSurfaceCapabilitiesKHR(vkSurface);
        config.imageCount = std::clamp(2u, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
        config.extent.width  = std::clamp(static_cast<uint32_t>(windowWidth),
                                           surfaceCapabilities.minImageExtent.width,
                                           surfaceCapabilities.maxImageExtent.width);
        config.extent.height = std::clamp(static_cast<uint32_t>(windowHeight),
                                            surfaceCapabilities.minImageExtent.height,
                                            surfaceCapabilities.maxImageExtent.height);

        return std::make_unique<SwapChain>(pContext, config);
    }

    SwapChain::SwapChain(const VulkanContext* pContext, SwapChainConfig config)
        : _pContext(pContext)
        , _swapChainConfig(config)
    {
        auto vkLogicalDevice = pContext->GetLogicalDevice();
        auto vkSurface = pContext->GetSurface();
        auto queueIndex = pContext->GetQueueIndex();

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

        if (queueIndex.graphicQueueIndex.value() == queueIndex.presentQueueIndex.value())
        {
            swapChainCreateInfo
                .setImageSharingMode(vk::SharingMode::eExclusive)
                .setQueueFamilyIndices(queueIndex.graphicQueueIndex.value());
        }
        else
        {
            std::array indices = {
                queueIndex.graphicQueueIndex.value(),
                queueIndex.presentQueueIndex.value()
            };
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
                .setLayerCount(1)   // number of mipmap levels (starting from baseMipLevel) accessible to the view
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
}
