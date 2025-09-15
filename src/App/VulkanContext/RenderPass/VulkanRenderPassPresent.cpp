#include "VulkanRenderPassPresent.h"
#include "VulkanContext/FrameBuffer/VulkanFrameBuffer.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"

namespace Ailurus
{
    
    VulkanRenderPassPresent::VulkanRenderPassPresent()
    {
        // Attachment
		vk::AttachmentDescription attachment;
		attachment.setFormat(VulkanContext::GetSwapChain()->GetConfig().surfaceFormat.format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eLoad)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		// Attachment ref
		vk::AttachmentReference colorRef;
		colorRef.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		// Subpass
		vk::SubpassDescription subpass;
		subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(colorRef);

		// Dependency
		vk::SubpassDependency dependency;
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask(vk::AccessFlagBits::eNone)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

		// Create info
		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo.setAttachments(attachment)
			.setSubpasses(subpass)
			.setDependencies(dependency);

		CreateRenderPass(renderPassInfo);
    }
    
    VulkanRenderPassPresent::~VulkanRenderPassPresent()
    {
    }
    
    RenderPassType VulkanRenderPassPresent::GetRenderPassType() const
    {
        return RenderPassType::Present;
    }
    
    vk::RenderPassBeginInfo VulkanRenderPassPresent::GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const
    {
        vk::RenderPassBeginInfo renderPassInfo;
		renderPassInfo.setRenderPass(GetRenderPass())
			.setFramebuffer(pTargetFrameBuffer->GetBuffer())
			.setRenderArea(vk::Rect2D{
				vk::Offset2D{ 0, 0 },
				VulkanContext::GetSwapChain()->GetConfig().extent });

		return renderPassInfo;
    }
}