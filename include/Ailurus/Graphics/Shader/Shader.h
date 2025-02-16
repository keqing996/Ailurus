#pragma once

#include <vulkan/vulkan.hpp>
#include "ShaderStage.h"

namespace Ailurus
{
    class Renderer;

    class Shader
    {
    public:
        Shader(const Renderer* pRenderer, ShaderStage stage, const std::string& path);
        Shader(const Renderer* pRenderer, ShaderStage stage, const char* binaryData, size_t size);
        ~Shader();

    public:
        ShaderStage GetStage() const;
        vk::ShaderModule GetShaderModule() const;
        vk::PipelineShaderStageCreateInfo GeneratePipelineCreateInfo() const;

    private:
        const Renderer* _pRenderer;

        ShaderStage _stage;
        vk::ShaderModule _vkShaderModule = nullptr;
    };
}