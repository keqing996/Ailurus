#pragma once

#include "VulkanContext/VulkanPch.h"
#include <Ailurus/Math/Matrix4x4.hpp>

namespace Ailurus
{
	struct StageShaderArray;
	class VulkanVertexLayout;
	class UniformSet;

	class VulkanPipeline
	{
	public:
		// Standard scene pipeline constructor (opaque geometry: depth write, no blending)
		// Pass blendEnabled=true and depthWriteEnabled=false for transparent geometry
		VulkanPipeline(vk::Format colorFormat, vk::Format depthFormat, const StageShaderArray& shaderArray,
			const VulkanVertexLayout* pVertexLayout, const std::vector<const UniformSet*>& uniformSets,
			uint32_t pushConstantSize = sizeof(Matrix4x4f),
			bool blendEnabled = false,
			bool depthWriteEnabled = true);

		// G-Buffer pipeline constructor: multiple color attachments, depth write, no blending
		VulkanPipeline(const std::vector<vk::Format>& colorFormats, vk::Format depthFormat,
			const StageShaderArray& shaderArray,
			const VulkanVertexLayout* pVertexLayout, const std::vector<const UniformSet*>& uniformSets,
			uint32_t pushConstantSize = sizeof(Matrix4x4f));

		// Post-process pipeline constructor: no vertex input, no depth, single sample, fragment push constants
		VulkanPipeline(vk::Format colorFormat, const StageShaderArray& shaderArray,
			const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
			uint32_t pushConstantSize = 0, bool blendEnabled = false);

		// Skybox pipeline constructor: no vertex input, depth test (read-only), MSAA-aware, vertex push constants
		VulkanPipeline(vk::Format colorFormat, vk::Format depthFormat,
			const StageShaderArray& shaderArray,
			const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
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
