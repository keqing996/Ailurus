#pragma once

#include <vulkan/vulkan.hpp>
#include "RHIPipeLineConfig.h"

namespace Ailurus
{
	class Pipeline
	{
	public:
		Pipeline(const class RenderPass* pRenderPass, const PipelineConfig& config);
		~Pipeline();

	public:
		vk::Pipeline GetPipeline() const;

	private:
		vk::PipelineLayout _vkPipelineLayout;
		vk::Pipeline _vkPipeline;
	};
} // namespace Ailurus
