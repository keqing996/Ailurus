#pragma once

#include "VulkanContext/VulkanPch.h"

namespace Ailurus
{
	struct StageShaderArray;
	class VulkanVertexLayout;
	class UniformSet;

	class VulkanPipeline
	{
	public:
		VulkanPipeline(vk::Format colorFormat, vk::Format depthFormat, const StageShaderArray& shaderArray, 
			const VulkanVertexLayout* pVertexLayout, const std::vector<const UniformSet*>& uniformSets,
			uint32_t pushConstantSize = sizeof(Matrix4x4f));
		~VulkanPipeline();

	public:
		vk::Pipeline GetPipeline() const;
		vk::PipelineLayout GetPipelineLayout() const;

	private:
		vk::PipelineLayout _vkPipelineLayout;
		vk::Pipeline _vkPipeline;
	};
} // namespace Ailurus
