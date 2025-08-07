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
		vk::RenderPassBeginInfo GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const override;

	private:
		vk::RenderPass _vkRenderPass;
	};
} // namespace Ailurus