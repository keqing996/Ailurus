#pragma once

#include <vulkan/vulkan.hpp>
#include "PipelineConfig.h"

namespace Ailurus
{
    class Render;
    class RenderPass;
    class Shader;

    class Pipeline
    {
    public:
        Pipeline(const Render* pRenderer, const RenderPass* pRenderPass, const PipelineConfig& config);
        ~Pipeline();

    public:
        vk::Pipeline GetPipeline() const;

    private:
        const Render* _pRenderer;

        vk::PipelineLayout _vkPipelineLayout;
        vk::Pipeline _vkPipeline;
    };
}
