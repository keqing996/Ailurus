#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "Vulkan/Pipeline/Pipeline.h"

namespace Ailurus
{
    class RenderPassRHI
    {
    public:
        virtual vk::RenderPass GetRenderPass() const = 0;
        virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo(const Flight& flight) = 0;
        const Pipeline* GetPipeline(const PipelineConfig& config);

    private:
        std::unordered_map<PipelineConfig, std::unique_ptr<Pipeline>, PipelineConfig::Hash> _pipelineMap;
    };
}