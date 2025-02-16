#include "Ailurus/Graphics/RenderPass/RenderPass.h"
#include "Ailurus/Graphics/Renderer.h"

namespace Ailurus
{
    RenderPass::RenderPass(const Renderer* pRenderer, const SwapChain* pSwapChain)
        : _pRenderer(pRenderer)
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

        _vkRenderPass = _pRenderer->GetLogicalDevice().createRenderPass(renderPassInfo);
    }

    RenderPass::~RenderPass()
    {
        _pRenderer->GetLogicalDevice().destroyRenderPass(_vkRenderPass);
    }

    vk::RenderPass RenderPass::GetRenderPass() const
    {
        return _vkRenderPass;
    }
}
