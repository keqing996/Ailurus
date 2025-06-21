#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	class VulkanPipeline
	{
	public:
		VulkanPipeline(const class VulkanRenderPass* pRenderPass, const class StageShaderArray& shaderArray, 
			const class VulkanVertexLayout* pVertexLayout, const std::vector<const class UniformSet*>& uniformSets);
		~VulkanPipeline();

	public:
		vk::Pipeline GetPipeline() const;
		vk::PipelineLayout GetPipelineLayout() const;

	private:
		vk::PipelineLayout _vkPipelineLayout;
		vk::Pipeline _vkPipeline;
	};
} // namespace Ailurus
