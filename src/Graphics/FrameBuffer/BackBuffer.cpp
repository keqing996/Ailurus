#include "Ailurus/Graphics/FrameBuffer/BackBuffer.h"

namespace Ailurus
{
    BackBuffer::BackBuffer(const VulkanContext* pContext, const SwapChain* pSwapChain, const RenderPass* pRenderPass): _pContext(pContext)
    {
        auto vkLogicalDevice = _pContext->GetLogicalDevice();
        auto extent = pSwapChain->GetSwapChainConfig().extent;
        auto& swapChainImageViews = pSwapChain->GetImageViews();
        for (auto i = 0; i < swapChainImageViews.size(); i++)
        {
            vk::FramebufferCreateInfo framebufferInfo;
            framebufferInfo.setRenderPass(pRenderPass->GetRenderPass())
                    .setAttachments(swapChainImageViews[i])
                    .setWidth(extent.width)
                    .setHeight(extent.height)
                    .setLayers(1);

            _vkFrameBuffer.push_back(vkLogicalDevice.createFramebuffer(framebufferInfo));
        }
    }

    BackBuffer::~BackBuffer()
    {
        for (auto frameBuffer: _vkFrameBuffer)
            _pContext->GetLogicalDevice().destroyFramebuffer(frameBuffer);
    }
}
