#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Application/RenderSystem/Enum/MultiSampling.h>
#include <Ailurus/Application/RenderSystem/Enum/StencilOperation.h>
#include <Ailurus/Application/RenderSystem/FrameBuffer/FrameBufferUsage.h>

#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/SwapChain/VulkanSwapChain.h>
#include <VulkanContext/Helper/VulkanHelper.h>

namespace Ailurus
{
	struct VulkanRenderPassConfig
	{
		struct FrameBufferOperation
		{
			// Usage
			FrameBufferUsage usage;

			// MSAA config
			MultiSamplingType multiSampling = MultiSamplingType::None;

			// Attachment
			bool clearAttachment = true;
			bool writeAttachment = true;

			// Stencil
			StencilLoadType stencilLoad = StencilLoadType::None;
			StencilWriteType stencilWrite = StencilWriteType::None;
		};

		


		vk::RenderPassCreateInfo GetVulkanRenderPassCreateInfo()
		{
			vk::AttachmentDescription colorAttachment;
			colorAttachment.setFormat(VulkanContext::GetSwapChain()->GetConfig().surfaceFormat.format)
				.setSamples(VulkanHelper::ConvertToVkEnum(multiSampling))
				.setLoadOp(clearAttachment ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
				.setStoreOp(writeAttachment ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare)
				.setStencilLoadOp(VulkanHelper::ConvertToVkEnum(stencilLoad))
				.setStencilStoreOp(VulkanHelper::ConvertToVkEnum(stencilWrite))
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