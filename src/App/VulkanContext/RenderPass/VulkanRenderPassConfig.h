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

		std::vector<FrameBufferOperation> frameBufferOperations;


		vk::RenderPassCreateInfo GetVulkanRenderPassCreateInfo() const
		{
			// 1. Build all attachments
			std::vector<vk::AttachmentDescription> attachments;
			std::vector<vk::AttachmentReference> colorAttachmentRefs;
			for (size_t i = 0; i < frameBufferOperations.size(); ++i)
			{
				const auto& fbOp = frameBufferOperations[i];
				vk::AttachmentDescription attachment;
				attachment.setFormat(VulkanContext::GetSwapChain()->GetConfig().surfaceFormat.format)
					.setSamples(VulkanHelper::ConvertToVkEnum(fbOp.multiSampling))
					.setLoadOp(fbOp.clearAttachment ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
					.setStoreOp(fbOp.writeAttachment ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare)
					.setStencilLoadOp(VulkanHelper::ConvertToVkEnum(fbOp.stencilLoad))
					.setStencilStoreOp(VulkanHelper::ConvertToVkEnum(fbOp.stencilWrite))
					.setInitialLayout(VulkanHelper::GetFrameBufferInitLayoutForRenderPass(fbOp.usage))
					.setFinalLayout(VulkanHelper::GetFrameBufferFinalLayoutForRenderPass(fbOp.usage));
				attachments.push_back(attachment);

				// Assume all are color attachments for now; extend as needed for depth/stencil
				vk::AttachmentReference colorRef;
				colorRef.setAttachment(static_cast<uint32_t>(i))
					.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
				colorAttachmentRefs.push_back(colorRef);
			}

			// 2. Support multiple subpasses if needed (here, single subpass for all color attachments)
			std::vector<vk::SubpassDescription> subpasses;
			vk::SubpassDescription subpass;
			subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
				.setColorAttachments(colorAttachmentRefs);
			subpasses.push_back(subpass);

			// 3. Support multiple dependencies if needed (here, single dependency for all subpasses)
			std::vector<vk::SubpassDependency> dependencies;
			vk::SubpassDependency dependency;
			dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
				.setDstSubpass(0)
				.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setSrcAccessMask(vk::AccessFlagBits::eNone)
				.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
			dependencies.push_back(dependency);

			vk::RenderPassCreateInfo renderPassInfo;
			renderPassInfo.setAttachments(attachments)
				.setSubpasses(subpasses)
				.setDependencies(dependencies);

			return renderPassInfo;
		}
	};
}