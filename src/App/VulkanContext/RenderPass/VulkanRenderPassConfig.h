#pragma once

#include <vulkan/vulkan.hpp>
#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/SwapChain/VulkanSwapChain.h>

namespace Ailurus
{
	struct VulkanRenderPassConfig
	{




		vk::RenderPassCreateInfo GetVulkanRenderPassCreateInfo()
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
		}
	};
}