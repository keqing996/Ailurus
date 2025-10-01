#include "VulkanRenderPassForward.h"
#include "VulkanContext/FrameBuffer/VulkanFrameBuffer.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"

namespace Ailurus
{

	VulkanRenderPassForward::VulkanRenderPassForward(const std::vector<vk::ClearValue>& clearValues)
		: _clearValues(clearValues)
	{
		// Attachment
		vk::AttachmentDescription attachment;
		attachment.setFormat(VulkanContext::GetSwapChain()->GetConfig().surfaceFormat.format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

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

	VulkanRenderPassForward::~VulkanRenderPassForward()
	{
	}

	RenderPassType VulkanRenderPassForward::GetRenderPassType() const
	{
		return RenderPassType::Forward;
	}

	vk::RenderPassBeginInfo VulkanRenderPassForward::GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const
	{
		vk::RenderPassBeginInfo renderPassInfo;
		renderPassInfo.setRenderPass(GetRenderPass())
			.setFramebuffer(pTargetFrameBuffer->GetBuffer())
			.setRenderArea(vk::Rect2D{
				vk::Offset2D{ 0, 0 },
				VulkanContext::GetSwapChain()->GetConfig().extent })
			.setClearValues(_clearValues);

		return renderPassInfo;
	}
} // namespace Ailurus