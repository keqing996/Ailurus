#pragma once

#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "VulkanRenderPassConfig.h"

namespace Ailurus
{
	class VulkanFrameBuffer;

	class VulkanRenderPass : public NonCopyable, public NonMovable
	{
	public:
		VulkanRenderPass(const VulkanRenderPassConfig& config, RenderPassType type, const std::vector<vk::ClearValue>& clearValues = {});
		~VulkanRenderPass();

		RenderPassType GetRenderPassType() const;
		vk::RenderPass GetRenderPass() const;
		vk::RenderPassBeginInfo GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const;

	private:
		vk::RenderPass _vkRenderPass;
		RenderPassType _type;
		std::vector<vk::ClearValue> _clearValues;
	};
} // namespace Ailurus