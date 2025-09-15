#include "VulkanRenderPass.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/FrameBuffer/VulkanFrameBuffer.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"

namespace Ailurus
{
	VulkanRenderPass::VulkanRenderPass()
	{
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		if (_vkRenderPass)
			VulkanContext::GetDevice().destroyRenderPass(_vkRenderPass);
	}

	vk::RenderPass VulkanRenderPass::GetRenderPass() const
	{
		return _vkRenderPass;
	}

	void VulkanRenderPass::CreateRenderPass(const vk::RenderPassCreateInfo& createInfo)
	{
		_vkRenderPass = VulkanContext::GetDevice().createRenderPass(createInfo);
	}
} // namespace Ailurus
