#pragma once

#include "VulkanContext/VulkanPch.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"

namespace Ailurus
{
	class VulkanFrameBuffer;

	class VulkanRenderPass : public NonCopyable, public NonMovable
	{
	public:
		VulkanRenderPass();
		virtual ~VulkanRenderPass();

		vk::RenderPass GetRenderPass() const;
		virtual RenderPassType GetRenderPassType() const = 0;
		virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const = 0;

	protected:
		void CreateRenderPass(const vk::RenderPassCreateInfo& createInfo);

	private:
		vk::RenderPass _vkRenderPass;
	};
} // namespace Ailurus