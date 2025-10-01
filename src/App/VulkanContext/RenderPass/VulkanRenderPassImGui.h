#pragma once

#include "VulkanRenderPass.h"

namespace Ailurus
{
	class VulkanRenderPassImGui : public VulkanRenderPass
	{
	public:
		VulkanRenderPassImGui();
		~VulkanRenderPassImGui() override;

	public:
		RenderPassType GetRenderPassType() const override;
		vk::RenderPassBeginInfo GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const override;
	};
}