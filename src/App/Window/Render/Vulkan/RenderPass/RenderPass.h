#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <unordered_map>

#include "RenderPassType.h"
#include "Ailurus/Graphics/Airport/Flight.h"
#include "Ailurus/Graphics/Pipeline/Pipeline.h"

namespace Ailurus
{
    class Render;
    class SwapChain;

    class RenderPass
    {
    public:
        explicit RenderPass(const Render* pRenderer) : _pRenderer(pRenderer) {}
        virtual ~RenderPass() = default;

    public:
        virtual RenderPassType GetRenderPassType() = 0;
        virtual vk::RenderPass GetRenderPass() const = 0;
        virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo(const Flight& flight) = 0;

        const Pipeline* GetPipeline(const PipelineConfig& config);

    protected:
        const Render* _pRenderer;

        std::unordered_map<PipelineConfig, std::unique_ptr<Pipeline>, PipelineConfig::Hash> _pipelineMap;
    };
}