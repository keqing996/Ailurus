#pragma once

#include "VulkanContext/VulkanPch.h"

namespace Ailurus
{
	class VulkanRenderPass;
	struct StageShaderArray;
	class VulkanVertexLayout;
	class UniformSet;

	class VulkanPipeline
	{
	public:
		VulkanPipeline(const VulkanRenderPass* pRenderPass, const StageShaderArray& shaderArray, 
			const VulkanVertexLayout* pVertexLayout, const std::vector<const UniformSet*>& uniformSets);
		~VulkanPipeline();

	public:
		vk::Pipeline GetPipeline() const;
		vk::PipelineLayout GetPipelineLayout() const;

	private:
		vk::PipelineLayout _vkPipelineLayout;
		vk::Pipeline _vkPipeline;
	};
} // namespace Ailurus
