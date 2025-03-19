#include "Ailurus/Application/RenderPass/RenderPassForward.h"
#include "Vulkan/Context/VulkanContext.h"
#include "Vulkan/SwapChain/SwapChain.h"

namespace Ailurus
{
    RenderPassForward::RenderPassForward(): RenderPass()
    {
        SetupRenderPass();
        SetupBackBuffers();
    }

    RenderPassForward::~RenderPassForward()
    {
        for (auto frameBuffer: _backBuffers)
            VulkanContext::GetDevice().destroyFramebuffer(frameBuffer);

        VulkanContext::GetDevice().destroyRenderPass(_vkRenderPass);
    }

    RenderPassType RenderPassForward::GetRenderPassType()
    {
        return RenderPassType::Forward;
    }

    vk::RenderPass RenderPassForward::GetRenderPass() const
    {
        return _vkRenderPass;
    }

    vk::RenderPassBeginInfo RenderPassForward::GetRenderPassBeginInfo(const Flight& flight)
    {
        static vk::ClearValue clearColor({0.0f, 0.0f, 0.0f, 1.0f});

        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.setRenderPass(_vkRenderPass)
                .setFramebuffer(_backBuffers[flight.imageIndex])
                .setRenderArea(vk::Rect2D{
                    vk::Offset2D{0, 0},
                    VulkanContext::GetSwapChain()->GetSwapChainConfig().extent
                })
                .setClearValues(clearColor);

        return renderPassInfo;
    }

    void RenderPassForward::SetupRenderPass()
    {
        vk::AttachmentDescription colorAttachment;
        colorAttachment.setFormat(VulkanContext::GetSwapChain()->GetSwapChainConfig().surfaceFormat.format)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        vk::AttachmentReference colorAttachmentRef;
        colorAttachmentRef.setAttachment(0)
                .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subpass;
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachments(colorAttachmentRef);

        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

        vk::RenderPassCreateInfo renderPassInfo;
        renderPassInfo.setAttachments(colorAttachment)
                .setSubpasses(subpass)
                .setDependencies(dependency);

        _vkRenderPass = VulkanContext::GetDevice().createRenderPass(renderPassInfo);
    }

    void RenderPassForward::SetupBackBuffers()
    {
        auto vkLogicalDevice = VulkanContext::GetDevice();
        auto pSwapChain = VulkanContext::GetSwapChain();
        auto extent = pSwapChain->GetSwapChainConfig().extent;
        auto& swapChainImageViews = pSwapChain->GetImageViews();
        for (auto i = 0; i < swapChainImageViews.size(); i++)
        {
            vk::FramebufferCreateInfo framebufferInfo;
            framebufferInfo.setRenderPass(_vkRenderPass)
                    .setAttachments(swapChainImageViews[i])
                    .setWidth(extent.width)
                    .setHeight(extent.height)
                    .setLayers(1);

            _backBuffers.push_back(vkLogicalDevice.createFramebuffer(framebufferInfo));
        }
    }
}
