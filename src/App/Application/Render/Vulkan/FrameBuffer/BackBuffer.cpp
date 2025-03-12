#include "Ailurus/Graphics/FrameBuffer/BackBuffer.h"
#include "Ailurus/Graphics/Renderer.h"

namespace Ailurus
{
    BackBuffer::BackBuffer(const Renderer* pRenderer, const SwapChain* pSwapChain, const RenderPass* pRenderPass)
        : _pRenderer(pRenderer)
    {
        auto vkLogicalDevice = _pRenderer->GetLogicalDevice();
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
            _pRenderer->GetLogicalDevice().destroyFramebuffer(frameBuffer);
    }

    const std::vector<vk::Framebuffer>& BackBuffer::GetBackBuffers()
    {
        return _vkFrameBuffer;
    }
}
