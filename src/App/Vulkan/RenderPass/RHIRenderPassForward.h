#pragma once

#include <vector>
#include "RHIRenderPass.h"

namespace Ailurus
{
	class RenderPassForward : public RHIRenderPass
	{
	public:
		RenderPassForward();
		~RenderPassForward() override;

	public:
		RenderPassType GetRenderPassType() override;
		vk::RenderPass GetRenderPass() const override;
		vk::RenderPassBeginInfo GetRenderPassBeginInfo(const Flight& flight) override;

	private:
		void SetupRenderPass();
		void SetupBackBuffers();

	private:
		vk::RenderPass _vkRenderPass;
		std::vector<vk::Framebuffer> _backBuffers;
	};
} // namespace Ailurus