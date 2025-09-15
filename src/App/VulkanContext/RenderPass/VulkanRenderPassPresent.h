#pragma once

#include "VulkanRenderPass.h"

namespace Ailurus
{
    class VulkanRenderPassPresent : public VulkanRenderPass
	{
	public:
		VulkanRenderPassPresent();
		~VulkanRenderPassPresent() override;

	public:
		RenderPassType GetRenderPassType() const override;
		vk::RenderPassBeginInfo GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const override;
	};
}