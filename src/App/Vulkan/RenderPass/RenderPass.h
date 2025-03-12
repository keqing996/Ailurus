#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Application/Render/RenderPass/RenderPassType.h"
#include "Vulkan/Airport/Flight.h"
#include "Vulkan/Pipeline/Pipeline.h"

namespace Ailurus
{
    class RenderPass
    {
    public:
        virtual ~RenderPass() = default;

    public:
        virtual RenderPassType GetRenderPassType() = 0;
        virtual vk::RenderPass GetRenderPass() const = 0;
        virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo(const Flight& flight) = 0;

        const Pipeline* GetPipeline(const PipelineConfig& config);

    protected:
        std::unordered_map<PipelineConfig, std::unique_ptr<Pipeline>, PipelineConfig::Hash> _pipelineMap;
    };
}