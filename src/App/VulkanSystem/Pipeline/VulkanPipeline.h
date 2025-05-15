#pragma once

#include <vulkan/vulkan.hpp>
#include "VulkanPipelineConfig.h"

namespace Ailurus
{
	class VulkanPipeline
	{
	public:
		VulkanPipeline(const class VulkanRenderPass* pRenderPass, const VulkanPipelineConfig& config);
		~VulkanPipeline();

	public:
		vk::Pipeline GetPipeline() const;

	private:
		vk::PipelineLayout _vkPipelineLayout;
		vk::Pipeline _vkPipeline;
	};
} // namespace Ailurus
