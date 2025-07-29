#include "VulkanRenderPassForward.h"
#include "Ailurus/Application/Application.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"

namespace Ailurus
{
	VulkanRenderPassForward::VulkanRenderPassForward()
	{
		SetupRenderPass();
		SetupBackBuffers();
	}

	VulkanRenderPassForward::~VulkanRenderPassForward()
	{
		for (auto frameBuffer : _backBuffers)
			VulkanContext::GetDevice().destroyFramebuffer(frameBuffer);

		VulkanContext::GetDevice().destroyRenderPass(_vkRenderPass);
	}

	RenderPassType VulkanRenderPassForward::GetRenderPassType()
	{
		return RenderPassType::Forward;
	}

	vk::RenderPass VulkanRenderPassForward::GetRenderPass() const
	{
		return _vkRenderPass;
	}

	vk::RenderPassBeginInfo VulkanRenderPassForward::GetRenderPassBeginInfo() const
	{
		static vk::ClearValue clearColor({ 0.0f, 0.0f, 0.0f, 1.0f });

		vk::RenderPassBeginInfo renderPassInfo;
		renderPassInfo.setRenderPass(_vkRenderPass)
			.setFramebuffer(_backBuffers[VulkanContext::GetCurrentParallelFrameIndex()])
			.setRenderArea(vk::Rect2D{
				vk::Offset2D{ 0, 0 },
				VulkanContext::GetSwapChain()->GetConfig().extent })
			.setClearValues(clearColor);

		return renderPassInfo;
	}

	void VulkanRenderPassForward::SetupRenderPass()
	{
		vk::AttachmentDescription colorAttachment;
		colorAttachment.setFormat(VulkanContext::GetSwapChain()->GetConfig().surfaceFormat.format)
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

	void VulkanRenderPassForward::SetupBackBuffers()
	{
		const auto vkLogicalDevice = VulkanContext::GetDevice();
		const auto extent = VulkanContext::GetSwapChain()->GetConfig().extent;
		auto& swapChainImageViews = VulkanContext::GetSwapChain()->GetSwapChainImageViews();
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
