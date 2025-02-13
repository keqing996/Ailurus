#pragma once

#include <vulkan/vulkan.hpp>
#include "ShaderStage.h"

namespace Ailurus
{
    class Shader
    {
    public:
        Shader(vk::Device logicDevice, ShaderStage stage, const std::string& path);
        Shader(vk::Device logicDevice, ShaderStage stage, const char* binaryData, size_t size);
        ~Shader();

    public:
        ShaderStage GetStage() const;
        vk::ShaderModule GetShaderModule() const;
        vk::PipelineShaderStageCreateInfo GeneratePipelineCreateInfo() const;

    private:
        vk::Device _vkLogicDevice;
        ShaderStage _stage;
        vk::ShaderModule _vkShaderModule = nullptr;
    };
}