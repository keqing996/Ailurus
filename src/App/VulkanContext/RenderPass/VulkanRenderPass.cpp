#include "VulkanRenderPass.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/FrameBuffer/VulkanFrameBuffer.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	VulkanRenderPass::VulkanRenderPass()
	{
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		try
		{
			if (_vkRenderPass)
				VulkanContext::GetDevice().destroyRenderPass(_vkRenderPass);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to destroy render pass: {}", e.what());
		}
	}

	vk::RenderPass VulkanRenderPass::GetRenderPass() const
	{
		return _vkRenderPass;
	}

	void VulkanRenderPass::CreateRenderPass(const vk::RenderPassCreateInfo& createInfo)
	{
		try
		{
			_vkRenderPass = VulkanContext::GetDevice().createRenderPass(createInfo);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create render pass: {}", e.what());
		}
	}
} // namespace Ailurus
