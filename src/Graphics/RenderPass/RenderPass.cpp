#include "Ailurus/Graphics/RenderPass/RenderPass.h"

namespace Ailurus
{
    vk::RenderPass RenderPass::GetRenderPass() const
    {
        return _vkRenderPass;
    }

    RenderPass::RenderPass(const VulkanContext* pContext, const SwapChain* pSwapChain)
        : _pContext(pContext)
    {
        vk::AttachmentDescription colorAttachment;
        colorAttachment.setFormat(pSwapChain->GetSwapChainConfig().surfaceFormat.format)
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

        _vkRenderPass = _pContext->GetLogicalDevice().createRenderPass(renderPassInfo);
    }

    RenderPass::~RenderPass()
    {
        _pContext->GetLogicalDevice().destroyRenderPass(_vkRenderPass);
    }

    std::unique_ptr<RenderPass> RenderPass::Create(const VulkanContext* pContext, const SwapChain* pSwapChain)
    {
        return std::unique_ptr<RenderPass>(new RenderPass(pContext, pSwapChain));
    }
}
