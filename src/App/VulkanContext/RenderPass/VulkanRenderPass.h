#pragma once

#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class VulkanFrameBuffer;

	class VulkanRenderPass : public NonCopyable, public NonMovable
	{
	public:
		VulkanRenderPass();
		virtual ~VulkanRenderPass();

		vk::RenderPass GetRenderPass() const;
		virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const = 0;

	protected:
		void CreateRenderPass(const vk::RenderPassCreateInfo& createInfo);

	private:
		vk::RenderPass _vkRenderPass;
	};
} // namespace Ailurus