#include "../../RenderSystem/RenderPass/VulkanRenderPass.h"

namespace Ailurus
{
	VulkanRenderPass::~VulkanRenderPass()
	{
	}

	const VulkanPipeline* VulkanRenderPass::GetPipeline(const VulkanPipelineConfig& config)
	{
		if (const auto itr = _pipelineMap.find(config); itr != _pipelineMap.end())
			return itr->second.get();

		auto pPipeline = std::make_unique<VulkanPipeline>(this, config);
		const auto pResult = pPipeline.get();
		_pipelineMap[config] = std::move(pPipeline);
		return pResult;
	}
} // namespace Ailurus
