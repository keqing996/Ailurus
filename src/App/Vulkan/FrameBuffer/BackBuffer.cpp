#include "BackBuffer.h"
#include "Ailurus/Application/RenderPass/RenderPass.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/RenderPass/RHIRenderPass.h"

namespace Ailurus
{
    BackBuffer::BackBuffer(const RenderPass* pRenderPass)
    {
        auto vkLogicalDevice = VulkanContext::GetDevice();
        auto extent = VulkanContext::GetSwapChainConfig().extent;
        auto& swapChainImageViews = VulkanContext::GetSwapChainImageViews();
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
            VulkanContext::GetDevice().destroyFramebuffer(frameBuffer);
    }

    const std::vector<vk::Framebuffer>& BackBuffer::GetBackBuffers()
    {
        return _vkFrameBuffer;
    }
}
