#pragma once

#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"

namespace Ailurus
{
	class VulkanFrameBuffer;

	class VulkanRenderPass : public NonCopyable, public NonMovable
	{
	public:
		virtual ~VulkanRenderPass();

	public:
		virtual RenderPassType GetRenderPassType() = 0;
		virtual vk::RenderPass GetRenderPass() const = 0;
		virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const = 0;

	};
} // namespace Ailurus