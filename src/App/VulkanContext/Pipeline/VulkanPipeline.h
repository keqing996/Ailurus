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
		// Standard scene pipeline constructor
		VulkanPipeline(vk::Format colorFormat, vk::Format depthFormat, const StageShaderArray& shaderArray,
			const VulkanVertexLayout* pVertexLayout, const std::vector<const UniformSet*>& uniformSets,
			uint32_t pushConstantSize = sizeof(Matrix4x4f));

		// Post-process pipeline constructor: no vertex input, no depth, single sample, fragment push constants
		VulkanPipeline(vk::Format colorFormat, const StageShaderArray& shaderArray,
			const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
			uint32_t pushConstantSize = 0, bool blendEnabled = false);

		~VulkanPipeline();

	public:
		vk::Pipeline GetPipeline() const;
		vk::PipelineLayout GetPipelineLayout() const;

	private:
		vk::PipelineLayout _vkPipelineLayout;
		vk::Pipeline _vkPipeline;
	};
} // namespace Ailurus
