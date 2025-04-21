#include "RHIRenderPassForward.h"
#include "Rhi/RhiContext.h"

namespace Ailurus
{
	RHIRenderPassForward::RHIRenderPassForward()
	{
		SetupRenderPass();
		SetupBackBuffers();
	}

	RHIRenderPassForward::~RHIRenderPassForward()
	{
		for (auto frameBuffer : _backBuffers)
			RhiContext::GetDevice().destroyFramebuffer(frameBuffer);

		RhiContext::GetDevice().destroyRenderPass(_vkRenderPass);
	}

	RenderPassType RHIRenderPassForward::GetRenderPassType()
	{
		return RenderPassType::Forward;
	}

	vk::RenderPass RHIRenderPassForward::GetRenderPass() const
	{
		return _vkRenderPass;
	}

	vk::RenderPassBeginInfo RHIRenderPassForward::GetRenderPassBeginInfo() const
	{
		static vk::ClearValue clearColor({ 0.0f, 0.0f, 0.0f, 1.0f });

		vk::RenderPassBeginInfo renderPassInfo;
		renderPassInfo.setRenderPass(_vkRenderPass)
			.setFramebuffer(_backBuffers[RhiContext::GetCurrentFrameIndex()])
			.setRenderArea(vk::Rect2D{
				vk::Offset2D{ 0, 0 },
				RhiContext::GetSwapChainConfig().extent })
			.setClearValues(clearColor);

		return renderPassInfo;
	}

	void RHIRenderPassForward::SetupRenderPass()
	{
		vk::AttachmentDescription colorAttachment;
		colorAttachment.setFormat(RhiContext::GetSwapChainConfig().surfaceFormat.format)
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

		_vkRenderPass = RhiContext::GetDevice().createRenderPass(renderPassInfo);
	}

	void RHIRenderPassForward::SetupBackBuffers()
	{
		const auto vkLogicalDevice = RhiContext::GetDevice();
		const auto extent = RhiContext::GetSwapChainConfig().extent;
		auto& swapChainImageViews = RhiContext::GetSwapChainImageViews();
		for (auto swapChainImageView : swapChainImageViews)
		{
			vk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.setRenderPass(_vkRenderPass)
				.setAttachments(swapChainImageView)
				.setWidth(extent.width)
				.setHeight(extent.height)
				.setLayers(1);

			_backBuffers.push_back(vkLogicalDevice.createFramebuffer(framebufferInfo));
		}
	}
} // namespace Ailurus
