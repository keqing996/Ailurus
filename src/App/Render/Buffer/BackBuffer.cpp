#include "BackBuffer.h"
#include "Ailurus/Application/Render/RenderPass/RenderPass.h"
#include "Render/Context/RhiContext.h"
#include "Render/RenderPass/RHIRenderPass.h"

namespace Ailurus
{
    BackBuffer::BackBuffer(const RenderPass* pRenderPass)
    {
        auto vkLogicalDevice = RhiContext::GetDevice();
        auto extent = RhiContext::GetSwapChainConfig().extent;
        auto& swapChainImageViews = RhiContext::GetSwapChainImageViews();
        for (auto i = 0; i < swapChainImageViews.size(); i++)
        {
            vk::FramebufferCreateInfo framebufferInfo;
            framebufferInfo.setRenderPass(pRenderPass->GetRHIRenderPass()->GetRenderPass())
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
            RhiContext::GetDevice().destroyFramebuffer(frameBuffer);
    }

    const std::vector<vk::Framebuffer>& BackBuffer::GetBackBuffers()
    {
        return _vkFrameBuffer;
    }
}
