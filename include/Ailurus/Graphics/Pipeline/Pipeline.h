#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Graphics/Shader/ShaderStage.h"

namespace Ailurus
{
    class Renderer;
    class RenderPass;
    class Shader;

    class Pipeline
    {
    public:
        Pipeline(const Renderer* pRenderer, const RenderPass* pRenderPass);
        ~Pipeline();

    public:
        void AddShader(Shader* pShader);
        void GeneratePipeline();
        vk::Pipeline GetPipeline() const;

    private:
        const Renderer* _pRenderer;
        const RenderPass* _pRenderPass;
        vk::PipelineLayout _vkPipelineLayout;
        vk::Pipeline _vkPipeline;
        std::unordered_map<ShaderStage, Shader*> _shaderMap;
    };
}
