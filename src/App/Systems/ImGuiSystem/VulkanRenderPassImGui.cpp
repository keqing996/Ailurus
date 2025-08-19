#include "VulkanRenderPassImGui.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"
#include "VulkanContext/FrameBuffer/VulkanFrameBuffer.h"

namespace Ailurus
{
    VulkanRenderPassImGui::VulkanRenderPassImGui()
	{
		vk::AttachmentDescription attachment = {};
        attachment.setFormat(VulkanContext::GetSwapChain()->GetConfig().surfaceFormat.format)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        vk::AttachmentReference colorAttachment = {};
        colorAttachment.setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subpass = {};
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachments(colorAttachment);

        vk::SubpassDependency dependency = {};
        dependency.setSrcSubpass(vk::SubpassExternal)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits::eNone)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

        vk::RenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.setAttachments(attachment)
            .setSubpasses(subpass)
            .setDependencies(dependency);

        _vkRenderPass = VulkanContext::GetDevice().createRenderPass(renderPassCreateInfo);
	}

	VulkanRenderPassImGui::~VulkanRenderPassImGui()
	{
		VulkanContext::GetDevice().destroyRenderPass(_vkRenderPass);
	}

	RenderPassType VulkanRenderPassImGui::GetRenderPassType()
	{
		return RenderPassType::ImGui;
	}

	vk::RenderPass VulkanRenderPassImGui::GetRenderPass() const
	{
		return _vkRenderPass;
	}

    vk::RenderPassBeginInfo VulkanRenderPassImGui::GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const
    {
        vk::ClearValue clearColor({ _clearColor.x, _clearColor.y, _clearColor.z, _clearColor.w });

        vk::RenderPassBeginInfo renderPassInfo;
		renderPassInfo.setRenderPass(_vkRenderPass)
			.setFramebuffer(pTargetFrameBuffer->GetBuffer())
			.setRenderArea(vk::Rect2D{
				vk::Offset2D{ 0, 0 },
				VulkanContext::GetSwapChain()->GetConfig().extent })
			.setClearValues(clearColor);

		return renderPassInfo;
    }
} // namespace Ailurus