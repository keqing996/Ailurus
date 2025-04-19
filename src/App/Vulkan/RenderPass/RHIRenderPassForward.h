#pragma once

#include <vector>
#include "RHIRenderPass.h"

namespace Ailurus
{
	class RHIRenderPassForward : public RHIRenderPass
	{
	public:
		RHIRenderPassForward();
		~RHIRenderPassForward() override;

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