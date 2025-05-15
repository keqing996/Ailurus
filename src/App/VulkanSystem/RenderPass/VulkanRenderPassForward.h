#pragma once

#include <vector>
#include "VulkanRenderPass.h"

namespace Ailurus
{
	class VulkanRenderPassForward : public VulkanRenderPass
	{
	public:
		VulkanRenderPassForward();
		~VulkanRenderPassForward() override;

	public:
		RenderPassType GetRenderPassType() override;
		vk::RenderPass GetRenderPass() const override;
		vk::RenderPassBeginInfo GetRenderPassBeginInfo() const override;

	private:
		void SetupRenderPass();
		void SetupBackBuffers();

	private:
		vk::RenderPass _vkRenderPass;
		std::vector<vk::Framebuffer> _backBuffers;
	};
} // namespace Ailurus