#include "Ailurus/Graphics/RenderPass/RenderPass.h"
#include "Ailurus/Graphics/Pipeline/Pipeline.h"
#include "Ailurus/Graphics/Renderer.h"

namespace Ailurus
{
    const Pipeline* RenderPass::GetPipeline(const PipelineConfig& config)
    {
        if (const auto itr = _pipelineMap.find(config); itr != _pipelineMap.end())
            return itr->second.get();

        auto pPipeline = std::make_unique<Pipeline>(_pRenderer, this, config);
        const auto pResult = pPipeline.get();
        _pipelineMap[config] = std::move(pPipeline);
        return pResult;
    }
}
