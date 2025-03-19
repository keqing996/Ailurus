#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Application/RenderPass/RenderPass.h"
#include "Vulkan/Pipeline/Pipeline.h"

namespace Ailurus
{
    class RenderPassRHI
    {
        std::unordered_map<PipelineConfig, std::unique_ptr<Pipeline>, PipelineConfig::Hash> pipelineMap;


        virtual vk::RenderPass GetRenderPass() const = 0;
        virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo(const Flight& flight) = 0;
        const Pipeline* GetPipeline(const PipelineConfig& config);
    };
}