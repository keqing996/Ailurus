#include "BackBuffer.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPass.h"
#include "VulkanSystem/VulkanSystem.h"
#include "RenderSystem/RenderPass/RHIRenderPass.h"

namespace Ailurus
{
    BackBuffer::BackBuffer(const RenderPass* pRenderPass)
    {
        auto vkLogicalDevice = Application::Get<VulkanSystem>()->GetDevice();
        auto extent = Application::Get<VulkanSystem>()->GetSwapChainConfig().extent;
        auto& swapChainImageViews = Application::Get<VulkanSystem>()->GetSwapChainImageViews();
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
        for (const auto frameBuffer: _vkFrameBuffer)
            Application::Get<VulkanSystem>()->GetDevice().destroyFramebuffer(frameBuffer);
    }

    const std::vector<vk::Framebuffer>& BackBuffer::GetBackBuffers()
    {
        return _vkFrameBuffer;
    }
}
