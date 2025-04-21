#pragma once

#include <vector>
#include "RhiRenderPass.h"

namespace Ailurus
{
	class RhiRenderPassForward : public RhiRenderPass
	{
	public:
		RhiRenderPassForward();
		~RhiRenderPassForward() override;

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