#include "VulkanRenderPass.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/FrameBuffer/VulkanFrameBuffer.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"

namespace Ailurus
{
	VulkanRenderPass::VulkanRenderPass(const VulkanRenderPassConfig& config, RenderPassType type, const std::vector<vk::ClearValue>& clearValues)
		: _type(type), _clearValues(clearValues)
	{
		auto renderPassInfo = config.GetVulkanRenderPassCreateInfo();
		_vkRenderPass = VulkanContext::GetDevice().createRenderPass(renderPassInfo);
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		VulkanContext::GetDevice().destroyRenderPass(_vkRenderPass);
	}

	RenderPassType VulkanRenderPass::GetRenderPassType() const
	{
		return _type;
	}

	vk::RenderPass VulkanRenderPass::GetRenderPass() const
	{
		return _vkRenderPass;
	}

	vk::RenderPassBeginInfo VulkanRenderPass::GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const
	{
		vk::RenderPassBeginInfo renderPassInfo;
		renderPassInfo.setRenderPass(_vkRenderPass)
			.setFramebuffer(pTargetFrameBuffer->GetBuffer())
			.setRenderArea(vk::Rect2D{
				vk::Offset2D{ 0, 0 },
				VulkanContext::GetSwapChain()->GetConfig().extent })
			.setClearValues(_clearValues);
		return renderPassInfo;
	}
} // namespace Ailurus
