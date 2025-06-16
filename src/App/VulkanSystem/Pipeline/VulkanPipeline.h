#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	class VulkanPipeline
	{
	public:
		VulkanPipeline(const class RenderPass* pRenderPass, const class Material* pMaterial, 
			const class VulkanVertexLayout* pVertexLayout);
		~VulkanPipeline();

	public:
		vk::Pipeline GetPipeline() const;

	private:
		vk::PipelineLayout _vkPipelineLayout;
		vk::Pipeline _vkPipeline;
	};
} // namespace Ailurus
