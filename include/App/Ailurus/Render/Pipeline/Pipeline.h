#pragma once

#include <vulkan/vulkan.hpp>
#include "PipelineConfig.h"

namespace Ailurus
{
    class Renderer;
    class RenderPass;
    class Shader;

    class Pipeline
    {
    public:
        Pipeline(const Renderer* pRenderer, const RenderPass* pRenderPass, const PipelineConfig& config);
        ~Pipeline();

    public:
        vk::Pipeline GetPipeline() const;

    private:
        const Renderer* _pRenderer;

        vk::PipelineLayout _vkPipelineLayout;
        vk::Pipeline _vkPipeline;
    };
}
