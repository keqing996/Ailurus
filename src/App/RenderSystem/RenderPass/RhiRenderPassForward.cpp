#include "RhiRenderPassForward.h"
#include "../Context/RhiContext.h"

namespace Ailurus
{
	RhiRenderPassForward::RhiRenderPassForward()
	{
		SetupRenderPass();
		SetupBackBuffers();
	}

	RhiRenderPassForward::~RhiRenderPassForward()
	{
		for (auto frameBuffer : _backBuffers)
			RhiContext::GetDevice().destroyFramebuffer(frameBuffer);

		VulkanSystem::GetDevice().destroyRenderPass(_vkRenderPass);
	}

	RenderPassType RhiRenderPassForward::GetRenderPassType()
	{
		return RenderPassType::Forward;
	}

	vk::RenderPass RhiRenderPassForward::GetRenderPass() const
	{
		return _vkRenderPass;
	}

	vk::RenderPassBeginInfo RhiRenderPassForward::GetRenderPassBeginInfo() const
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

	void RhiRenderPassForward::SetupRenderPass()
	{
		vk::AttachmentDescription colorAttachment;
		colorAttachment.setFormat(VulkanSystem::GetSwapChainConfig().surfaceFormat.format)
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

		_vkRenderPass = VulkanSystem::GetDevice().createRenderPass(renderPassInfo);
	}

	void RhiRenderPassForward::SetupBackBuffers()
	{
		const auto vkLogicalDevice = VulkanSystem::GetDevice();
		const auto extent = VulkanSystem::GetSwapChainConfig().extent;
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
