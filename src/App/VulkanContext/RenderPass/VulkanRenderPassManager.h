#pragma once

#include <unordered_map>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "VulkanRenderPass.h"

namespace Ailurus
{
	class VulkanRenderPassManager: public NonCopyable, public NonMovable
	{
	public:
		VulkanRenderPassManager();
		~VulkanRenderPassManager();

	private:
		std::unordered_map<RenderPassType, std::unique_ptr<VulkanRenderPass>> _renderPassMap;
	};
}