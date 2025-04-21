#pragma once

#include <vulkan/vulkan.hpp>
#include "RHIPipelineConfig.h"

namespace Ailurus
{
	class RhiPipeline
	{
	public:
		RhiPipeline(const class RhiRenderPass* pRenderPass, const RhiPipelineConfig& config);
		~RhiPipeline();

	public:
		vk::Pipeline GetPipeline() const;

	private:
		vk::PipelineLayout _vkPipelineLayout;
		vk::Pipeline _vkPipeline;
	};
} // namespace Ailurus
