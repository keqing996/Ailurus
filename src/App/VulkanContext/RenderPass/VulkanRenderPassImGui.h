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
		vk::RenderPassBeginInfo GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const override;
	};
}