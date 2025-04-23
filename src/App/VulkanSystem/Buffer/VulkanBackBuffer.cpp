#include "VulkanBackBuffer.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPass.h"
#include "VulkanSystem/VulkanSystem.h"
#include "../RenderPass/VulkanRenderPass.h"

namespace Ailurus
{
    VulkanBackBuffer::VulkanBackBuffer(const RenderPass* pRenderPass)
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

    VulkanBackBuffer::~VulkanBackBuffer()
    {
        for (const auto frameBuffer: _vkFrameBuffer)
            Application::Get<VulkanSystem>()->GetDevice().destroyFramebuffer(frameBuffer);
    }

    const std::vector<vk::Framebuffer>& VulkanBackBuffer::GetBackBuffers()
    {
        return _vkFrameBuffer;
    }
}
