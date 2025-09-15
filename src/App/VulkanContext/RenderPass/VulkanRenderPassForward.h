#pragma once

#include "VulkanRenderPass.h"

namespace Ailurus
{
	class VulkanRenderPassForward : public VulkanRenderPass
	{
	public:
		VulkanRenderPassForward(const std::vector<vk::ClearValue>& clearValues);
		~VulkanRenderPassForward() override;

	public:
		RenderPassType GetRenderPassType() const override;
		vk::RenderPassBeginInfo GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const override;

	private:
		std::vector<vk::ClearValue> _clearValues;
	};
}