#include "RhiRenderPass.h"

namespace Ailurus
{
	RhiRenderPass::~RhiRenderPass()
	{
	}

	const RhiPipeline* RhiRenderPass::GetPipeline(const RhiPipelineConfig& config)
	{
		if (const auto itr = _pipelineMap.find(config); itr != _pipelineMap.end())
			return itr->second.get();

		auto pPipeline = std::make_unique<RhiPipeline>(this, config);
		const auto pResult = pPipeline.get();
		_pipelineMap[config] = std::move(pPipeline);
		return pResult;
	}
} // namespace Ailurus
