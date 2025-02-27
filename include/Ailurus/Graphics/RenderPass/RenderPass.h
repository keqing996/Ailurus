#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <unordered_map>
#include "RenderPassType.h"
#include "Ailurus/Graphics/Airport/Flight.h"
#include "Ailurus/Graphics/Pipeline/PipelineConfig.h"

namespace Ailurus
{
    class Renderer;
    class SwapChain;
    class Pipeline;

    class RenderPass
    {
    public:
        explicit RenderPass(const Renderer* pRenderer) : _pRenderer(pRenderer) {}
        virtual ~RenderPass() = 0;

    public:
        virtual vk::RenderPass GetRenderPass() const = 0;
        virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo(const Flight& flight) = 0;

        const Pipeline* GetPipeline(const PipelineConfig& config);

    protected:
        const Renderer* _pRenderer;

        std::unordered_map<PipelineConfig, std::unique_ptr<Pipeline>, PipelineConfig::Hash> _pipelineMap;
    };
}