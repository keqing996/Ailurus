#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "../Shader/Shader.h"

namespace Ailurus
{
    class Pipeline
    {
    public:
        Pipeline(vk::Device logicDevice, vk::RenderPass pass);
        ~Pipeline();

    public:
        void AddShader(ShaderStage stage, Shader* pShader);
        void GeneratePipeline();

    private:
        vk::Device _vkLogicDevice;
        vk::RenderPass _vkRenderPass;
        vk::PipelineLayout _vkPipelineLayout;
        vk::Pipeline _vkPipeline;

        std::unordered_map<ShaderStage, Shader*> _shaderMap;
    };
}
