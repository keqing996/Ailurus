#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "../Shader/Shader.h"
#include "../Context/VulkanContext.h"
#include "../RenderPass/RenderPass.h"

namespace Ailurus
{
    class Pipeline
    {
    public:
        Pipeline(const VulkanContext* pContext, const RenderPass* pRenderPass);
        ~Pipeline();

    public:
        void AddShader(Shader* pShader);
        void GeneratePipeline();

    private:
        const VulkanContext* _pContext;
        const RenderPass* _pRenderPass;
        vk::PipelineLayout _vkPipelineLayout;
        vk::Pipeline _vkPipeline;
        std::unordered_map<ShaderStage, Shader*> _shaderMap;
    };
}
